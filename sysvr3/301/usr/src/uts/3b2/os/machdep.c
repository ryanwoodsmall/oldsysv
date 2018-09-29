/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/machdep.c	10.15"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/csr.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sit.h"
#include "sys/pcb.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/sbd.h"
#include "sys/dma.h"
#include "sys/utsname.h"
#include "sys/acct.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/edt.h"
#include "sys/firmware.h"
#include "sys/var.h"
#include "sys/message.h"
#include "sys/inline.h"
#include "sys/conf.h"

/*	The following flags are used to indicate the cause
**	of a level 8 interrupt.
*/

char	iswtchflag;

/*	The following flags are used to indicate the
**	cause of a level 9 (PIR) interrupt.
*/

char	timeflag;
char	uartflag;
char	pwrflag;

/*	The follow access various locations in the firmware
**	vector table or edt.
**
**	The number of entries in the edt.
*/

#define	VNUM_EDT	(*(((struct vectors *)VBASE)->p_num_edt))

/*	A pointer to the start of the edt.
*/

#define	VP_EDT		(((struct vectors *)VBASE)->p_edt)

/*	A pointer to an entry in the edt.
*/

#define	V_EDTP(X)	(VP_EDT + (X))


/*
 * Start clock
 * This routine must run at IPL15
 */

clkstart()
{
	sitcmd(ITINIT, ITLSB, ITMSB);		/* init interval timer */
}

/*
 * SIT communication routine
 */

sitcmd(cmd, lsb, msb)
register unsigned char	cmd;	/* Command.			*/
register unsigned char	lsb;	/* Least significant byte.	*/
register unsigned char	msb;	/* Most significant byte.	*/
{
	extern struct sit sbdpit;

	register struct sit	*sitp;
	register unsigned char	*count;

	sitp = &sbdpit;

	switch (cmd & SITSC) {		/* decode counter select */
	    case SITCT0:
		count = &(sitp->count0);
		break;
	    case SITCT1:
		count = &(sitp->count1);
		break;
	    case SITCT2:
		count = &(sitp->count2);
		break;
	    case SITILL:
		cmn_err(CE_PANIC,"Illegal SIT counter selected");
	}
	switch (cmd & SITRL) {		/* decode read/load select */
	    case SITLAT:
		sitp->command = cmd;
		return(*count);
	    case SITLSB:
		sitp->command = cmd;
		*count = lsb;
		break;
	    case SITMSB:
		sitp->command = cmd;
		*count = msb;
		break;
	    case SITLMB:
		sitp->command = cmd;
		*count = lsb;
		*count = msb;
	}
	return(0);			/* keep lint(1) happy */
}

/*
 * Adjust the time to UNIX based time
 * This routine must run at IPL15
 */

clkset(oldtime)
register time_t	oldtime;
{
	time = (unsigned)oldtime;
}

/*
 * Stop the clock.
 * This routine must run at IPL15
 */

clkreld()
{
	extern struct sit sbdpit;

	/*	Stop the interval timer.
	*/

	((struct sit *)(&sbdpit))->command = ITINIT ;
}

/*
 * Request a PIR9
 */
extern	int	sbdwcsr ;

timepoke()
{
	timeflag = 1 ;
	((struct wcsr *)(&sbdwcsr))->s_pir9 = 0x1 ;
}


/*
 * Send an interrupt to process
 */

psw_t	sendsig_psw = SIGPSW;

sendsig(hdlr)
register int	(* hdlr)();
{
	struct x {
		int	(*pc)();
		psw_t	psw;
	}		stackframe;
	psw_t ill_psw;

	if (u.u_pcb.sp + sizeof(stackframe) >= u.u_pcb.sub)
		if (!grow(u.u_pcb.sp + sizeof(stackframe)))
			return;
	
	stackframe.pc  = u.u_pcb.pc;
	ill_psw = u.u_pcb.psw;
	ill_psw.CM = 01;
	stackframe.psw = ill_psw;
	if (copyout(&stackframe, u.u_pcb.sp, sizeof(struct x)) < 0)
		return;
	u.u_pcb.sp += sizeof(stackframe) / NBPW;

	u.u_pcb.pc   =  hdlr;
	u.u_pcb.psw  =  sendsig_psw;
	u.u_pcb.psw.OE = ill_psw.OE;
}


/*	The following table is indexed by the dma channel
**	number to find the address of the page register.
*/

struct r8	*dmapreg[4] = {
	(struct r8  *)(&dmaid),	/* Integral disk page reg. addr.   */
	(struct r8  *)(&dmaif),	/* Integral floppy page reg. addr. */
	(struct r8  *)(&dmaiuA),/* Integral uart A page reg. addr. */
	(struct r8  *)(&dmaiuB),/* Integral uart B page reg. addr. */
};


/*	Initialize the DMA controller.
*/

dmainit()
{
	/*	Init the dmac signal level sense.
	*/

	dmac.RSR_CMD = RSTDMA;
}


/*
 *  Routine used by the integral disk, integral floppy, and
 *  the integral UARTs to access the DMAC.
 */

dma_access(chan, baddr, numbytes, mode, cmd)
register unsigned char chan, mode, cmd;
register unsigned int baddr, numbytes;
{
	register unsigned char	temp;
	register unsigned char	*chanselp;
	register		s;

	/* raise priority level and save original */

	s = spltty();

	/* load memory base address into dmac */

	chanselp = &dmac.C0CA;	/* Init channel select pointer.	*/
	dmac.CBPFF = NULL;	/* init dmac byte ptr flip-flop */

	chanselp[chan*2] = (unsigned char)(baddr&0xff);   /*load byte3*/
	chanselp[chan*2] = (unsigned char)((baddr>>8)&0xff); /*load byte2*/
        temp = (unsigned char)((baddr>>16)&0xff);

	if (cmd >> 3)
		temp |= 0x80;
	else
		temp |= 0x00;
        dmapreg[chan]->data = temp;  	  /*load byte1*/

	/* byte 0 is always 2000000 hex so it need not be loaded */

	/* load byte count into dmac */

	dmac.CBPFF = NULL;	/* reinit dmac byte ptr flip-flop */
	numbytes -= 1;
	chanselp[chan*2+1] = (unsigned char)(numbytes & 0xff); /*load byte3*/
	chanselp[chan*2+1] = (unsigned char)((numbytes>>8)&0xff);/*load byte2*/

	/* load transfer mode, command type, and channel into dmac,
	   enable the channel, and restore interrupt priority level */

	dmac.WMODR = (mode | cmd | chan);
	dmac.WMKR = chan;
	splx(s);
}


/*
 * Clear registers on exec
 */

setregs()
{
	register int	i;
	void (* *rp)();

	u.u_procp->p_chold = 0;	/* clear p_chold */

	/* Any pending signal remain held, so don't clear p_hold and
	p_sig */	

	/* If the action was to catch the signal, then the action
	must be reset to SIG_DFL */

	for (rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if (*rp != SIG_IGN)
			*rp = SIG_DFL;


	u.u_ar0[PC] = u.u_exdata.ux_entloc;

	/* Clear illegal opcode handler */
	u.u_iop = NULL;

	for (i=0; i<v.v_nofiles; i++)
		if ((u.u_pofile[i] & EXCLOSE) && u.u_ofile[i] != NULL) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
		}
}

/* Allocate 'size' units from the given map,
 * returning the base of an area which starts on a "mask" boundary.
 * That is, which has all of the bits in "mask" off in the starting
 * address.  Mask is assummed to be (2**N - 1).
 * Algorithm is first-fit.
 */

ptmalloc(mp, size, mask)
struct map *mp;
{
	register int a, b;
	register int gap, tail;
	register struct map *bp;

	ASSERT(size >= 0);
	for (bp = mapstart(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			b = (a+mask) & ~mask;
			gap = b - a;
			if (bp->m_size < (gap + size))
				continue;
			if (gap != 0) {
				tail = bp->m_size - size - gap;
				bp->m_size = gap;
				if (tail) 
					mfree(mp, tail, bp->m_addr+gap+size);
			} else {
				bp->m_addr += size;
				if ((bp->m_size -= size) == 0) {
					do {
						bp++;
						(bp-1)->m_addr = bp->m_addr;
					} while ((bp-1)->m_size = bp->m_size);
					mapsize(mp)++;
				}
			}
			ASSERT(bp->m_size < 0x80000000);
			return(b);
		}
	}
	return(0);
}


/*
 * Copy to a newly allocated page
 * Both frompfn and topfn are given as page frame numbers
 */

copyseg(frompfn, count, topfn)
register int frompfn;
register int count;	/* in clicks */
register int topfn;
{
	register caddr_t from;
	register caddr_t to;

	from = (caddr_t) (pfntokv(frompfn));
	to = (caddr_t) (pfntokv(topfn));

	fbcopy(from, to, count);
}

/*
 * Determine (kernel) virtual address of the sde_t needed for translation
 * of virtual address within the address space of current process
 *
 *	va   --> virtual address
 */

sde_t *
vatosde(va)
register unsigned va;
{
	register sde_t	*sde;
	register proc_t	*p;
	register int	sid;
	union {
		unsigned intvar;
		VAR vaddr;
	} virtaddr;

	virtaddr.intvar = va;
	sid = virtaddr.vaddr.v_sid;

	/*
	 * Set sde to base of required SDT.
	 */

	switch (sid) {
		case 0:
		case 1:
			sde = (sde_t *) phystokv(srama[sid]);
			break;
		case 2:
		case 3:
			p = u.u_procp;
			sde = (sde_t *) phystokv(p->p_srama[sid - SCN2]);
			break;
	};
	sde += virtaddr.vaddr.v_ssl;

	return(sde);
}


/*
 * Determine (kernel) virtual address of the pde_t needed for translation
 * of virtual address within the address space of current process
 *
 *	va   --> virtual address
 *	sde  --> (kernel) virtual address of segment descriptor used for
 *		 translating va
 */

pde_t *
vatopde(va,sde)
register unsigned va;
register sde_t *sde;
{
	register pde_t *pde;
	union {
		unsigned intvar;
		VAR vaddr;
	} virtaddr;

	virtaddr.intvar = va;

	pde = (pde_t *) phystokv(sde->wd2.address) + virtaddr.vaddr.v_psl;
	return(pde);
}

/* Routine to translate virtual addresses to physical addresses
 * Used by the floppy, Integral, and Duart driver to access the dma
 * If an invalid address is received vtop returns a 0
 */

#define EBADDR	0

extern struct proc *curproc;

paddr_t
vtop(vaddr,procno)
struct proc *procno;
register long vaddr;
{
	register paddr_t	retval;
	register sde_t		*sp;
	register sde_t		*sdp;
	register pde_t		*pdp;
	register int		sid;
	register 		s;
	paddr_t			svirtophys();

	s = splhi();
	sid = secnum(vaddr);
	if ((sid == 0) || (sid == 1) || (procno == curproc)) {
		splx(s);
		return(svirtophys(vaddr));
	} else  {
		sp = (sde_t *)phystokv(procno->p_srama[sid - SCN2]);
		sdp = &sp[sgnum(vaddr)];
		if (indirect(sdp))
			sdp = (sde_t *)(sdp->wd2.address);

		/* perform validity check */

		if ((sdp->seg_flags & SDE_flags) != SDE_flags) {
			splx (s);
			return((paddr_t)EBADDR);
		}

		/* perform maximum offset check */

		if ((vaddr & MSK_IDXSEG) > motob(sdp->seg_len)) {
			splx (s);
			return((paddr_t)EBADDR); 
		}

		if (iscontig(sdp)) {
			retval = (paddr_t)((long)(sdp->wd2.address & ~0x7) +
				 (vaddr & MSK_IDXSEG));
		} else {
			pdp = (pde_t *)((int)sdp->wd2.address & ~0x1f);
			pdp = &pdp[(vaddr >> 11) & 0x3f];
			if (!pdp->pgm.pg_pres) {
				splx(s);
				return(EBADDR);
			}
			retval = ctob(pdp->pgm.pg_pfn) + poff(vaddr);
		}
		splx (s);
		return (retval);
	} 
}

paddr_t
svirtophys(vaddr)
int vaddr;
{
	register int	retval;
	register int	prio;
	register int	tempaddr;
	register int	caddrsave;
	int		svirtophyserr();

	prio = splhi();
	tempaddr = vaddr;
	vaddr = (tempaddr & ~3);
	caddrsave = u.u_caddrflt;
	u.u_caddrflt = (int)svirtophyserr;
	asm("	MOVTRW *0(%ap),%r8");
	asm("	NOP");
	u.u_caddrflt = caddrsave;
	retval |= (tempaddr & 3);
	splx(prio);
	return((paddr_t) retval);
}

svirtophysfail()
{
	asm("	.globl	svirtophyserr");
	asm("svirtophyserr:	");
	cmn_err(CE_PANIC, "svirtophys - movtrw failed.");
}
 

devcheck(devtype, dev_addr)
int devtype;
paddr_t dev_addr[];
{
	int dev_cnt;
	int i;
 
	dev_cnt = 0;
	for (i=0; i < VNUM_EDT; i++)
	{
		if (V_EDTP(i)->opt_code == devtype)
			dev_addr[dev_cnt++] =
			   (paddr_t)((17 * V_EDTP(i)->opt_slot - 14) * NBPS);
	}

	return(dev_cnt);
}

unsigned char
getvec(baddr)
register long baddr;
{
	/*	Simulate the system routine that will supply the
	**	interrupt vector given a peripheral board address.
	*/

	return(((((baddr / 131072) - 3) / 17) + 1) << 4);
}

