/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sys3b.c	10.19"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/iu.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/edt.h"
#include "sys/firmware.h"
#include "sys/sys3b.h"
#include "sys/nvram.h"
#include "sys/utsname.h"
#include "sys/sysmacros.h"
#include "sys/boothdr.h"
#include "sys/uadmin.h"
#include "sys/map.h"
#include "sys/inode.h"
#include "sys/swap.h"
#include "sys/gate.h"
#include "sys/var.h"
#include "sys/pfdat.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"
#include "sys/buf.h"

#define	FWREL(ptr)	((caddr_t) (ptr) + VROM)

#define VMEMINIT	(*(((struct vectors *)VBASE)->p_meminit))
#define VMEMSIZE	(*(((struct vectors *)VBASE)->p_memsize))
#define VSERNO		((struct serno *) \
			FWREL(((struct vectors *)VBASE)->p_serno))

#define	VCHKNVRAM	((char (*)()) \
			FWREL(((struct vectors *)VBASE)->p_chknvram))
#define	VRNVRAM		((char (*)()) \
			FWREL(((struct vectors *) VBASE)->p_rnvram))
#define	VWNVRAM		((char (*)()) \
			FWREL(((struct vectors *) VBASE)->p_wnvram))
#define	VDEMON		((char (*)()) \
			FWREL(((struct vectors *) VBASE)->p_demon))
 
/* pointer to equipped device table */

#define VP_EDT		(((struct vectors *)VBASE)->p_edt)
#define	V_EDTP(x)	(VP_EDT + x)

/* pointer to number of edt entries */

#define VNUM_EDT	(*(((struct vectors *)VBASE)->p_num_edt))

/*	The following are the PC and PSW which demon set into
**	the second level gate table entry for a bpt interrupt
**	when you set a breakpoint.  These defines are temporary.
**	They will eventually be put into the vector table.
*/

extern struct s3bsym  sys3bsym;		/* generated symbol table	 */
extern struct s3bconf sys3bconfig;	/* generated configuration table */
extern struct s3bboot sys3bboot;	/* generated boot program name	 */
					/* and timestamp		 */

psw_t	gatepsw = GPSW;

/*	The following is the control character which will allow
**	us to enter demon.  See code in iu.c.
*/

int	Demon_cc = 0x10;	/* Cntrl-p is the default.	*/

/*	The following is for floating point	*/

extern int mau_present;			/* mau equipped? */

/*	The following are used by the S3BDELMEM and S3BADDMEM
**	functions.
*/

pfd_t	Delmem;		/* Linked list of deleted pages.	*/
int	Delmem_cnt;	/* Count of number of deleted pages.	*/

/*	The page table for the gate table.
*/

extern int	*gateptbl0;

#define DMMSEG	102			/* segments 102  through  114  */
#define	DMMADDR	(VSECT2 + stob(DMMSEG))

/*
 *	3B System Call
 */

int sys3bautoconfig=0;			/* auto-config flag: 0-no 1-yes */
int dbug;
caddr_t dmmatt();


sys3b()
{
	register  struct  a {
		int	cmd;
		int	arg1, arg2, arg3;
	} *uap = (struct a *) u.u_ap;

	register int	idx;
	register int	c;

	struct nvparams	nvpx;
	struct todc	clkx;
	char		sysnamex[sizeof(utsname.sysname)];

	extern rtodc(), wtodc();
	extern unsigned char opcrmask;

	switch (uap->cmd) {
	case 23:
		switch (uap->arg1) {
			case 0:
				break;
			case 1:
				dbug = uap->arg2;
				break;
			default:
				u.u_error = EINVAL;
		}
		u.u_rval1 = dbug;
		break;

	case GRNON:	/* turn green light to a solid on state */
		if (suser())
			greenled(0);
		break;

	case GRNFLASH:	/* starts green light flashing */
		if (suser())
			greenled((HZ * 27) / 100);
		break;


	case S3BNVPRT:	/* print an xtra_nvr structure */
		if (!suser())
			break;
		if ( ! userdma(uap->arg1, sizeof(struct xtra_nvr), B_WRITE) )
			u.u_error = EFAULT;
		else {
			dumpnvram((struct xtra_nvr *)uap->arg1) ;
			undma(uap->arg1, sizeof(struct xtra_nvr), B_WRITE);
		}
		break;

	/*
 	 * Copy a string from a given kernel address to a user buffer. Uses
	 * copyout() for each byte to trap bad kernel addresses while watching
	 * for the null terminator.
	 */
	case S3BKSTR :
	{
		register char	*src;
		register char	*dst;
		register char	*dstlim;

		if (!suser())
			break;
		src = (char *) uap->arg1;
		dstlim = (dst = (char *) uap->arg2) + (unsigned int) uap->arg3;
		do {
			if (dst == dstlim || copyout(src, dst++, 1) < 0) {
				u.u_error = EINVAL;
				return (-1) ;
			}
		} while (*src++);
		return (0) ;
	}

	/* acquire generated system symbol table or config table
	 * sys3b(S3BSYM, &buffer, sizeof(buffer))
	 * sys3b(S3BCONF, &buffer, sizeof(buffer))
	 */
	case S3BSYM:
	case S3BCONF:
		{
                register int *buffer = (int*)(uap->arg1);
                register int size = uap->arg2;
		register int *dataddr;
		register int datalen;

		switch (uap->cmd) {
			case S3BSYM:
			    dataddr = (int*)&sys3bsym;
			    datalen = sys3bsym.size-sizeof(sys3bsym.size);
			    break;
			case S3BCONF:
		 	    dataddr = (int*)&sys3bconfig;
			    datalen = sys3bconfig.count*sizeof(struct s3bc);
			    break;
		}

		if (size < sizeof(int))
			{
			/* buffer too small even for size */
			u.u_error = EINVAL;
			break;
			}

		if (suword(buffer,*dataddr) == -1)
			{
			/* bad buffer address */
			u.u_error = EFAULT;
			break;
			}

		if ((size -= sizeof(int)) <= 0)
			break;

		if (size > datalen)
			size = datalen;

		if (copyout(dataddr+1,buffer+1,size) == -1)
			/* bad buffer address */
			u.u_error = EFAULT;

		break;
		}

	/*
	 * acquire timestamp and path name of boot program
	 *
	 * sys3b(S3BBOOT, &buffer)
	 */
	case S3BBOOT:
		if (copyout(&sys3bboot,uap->arg1,sizeof(struct s3bboot)) == -1)
			/* bad buffer address */
			u.u_error = EFAULT;
		break;

	/*
	 * Was an auto-config boot done?  Return 1 if yes, 0 if no.
	 *
	 * sys3b(S3BAUTO)
	 */
	case S3BAUTO:
		if (!suser())
			break;
		u.u_rval1 = sys3bautoconfig;
		break;


	/*
	 * undocumented fast illegal op-code handler interface for floating
	 * point simulation
	 *
	 * sys3b(S3BIOP, handler)
	 */

	case S3BIOP : {
		register preg_t	*prp;

		if (uap->arg1 == 0) {
			/* disable the fast interface */
			u.u_iop = NULL;
			break;
		}

		prp = findpreg(u.u_procp, PT_TEXT);
		if (prp == NULL  ||  (caddr_t)uap->arg1 < prp->p_regva  ||
		   (caddr_t)uap->arg1 >=
				(prp->p_regva + ctob(prp->p_reg->r_pgsz))) {
			/* bad handler address */

			u.u_error = EFAULT;
		} else {
			if (u.u_procp->p_flag & STRC)
				/* not supported while tracing to
				** preserve SDB's sanity
				*/

				u.u_error = EINVAL;
			else
				u.u_iop = (int*)uap->arg1;
		}
		break;
	}

	case RTODC:	/* read TODC */
		if (!suser())
			break;
		rtodc(&clkx);
		if (copyout((caddr_t) &clkx, (caddr_t) uap->arg1, sizeof(clkx)))
			u.u_error = EFAULT;
		break;

	case STIME:	/* set internal time, not hardware clock */
		if (!suser())
			break;
		time = (time_t) uap->arg1;
		break;

 
	case SETNAME:	/* rename the system */
		if (!suser())
			break;
		for (idx = 0;
		  (c = fubyte((caddr_t) uap->arg1 + idx)) > 0
		    && idx < sizeof(sysnamex) - 1;
		  ++idx)
			sysnamex[idx] = c;
		if (c) {
			u.u_error = c < 0 ? EFAULT : EINVAL;
			break;
		}
		sysnamex[idx] = '\0';
		str8cpy(utsname.sysname, sysnamex);
		str8cpy(utsname.nodename, sysnamex);
		break;
 
	case WNVR:	/* write NVRAM */
		if (!suser())
			break;
		if (copyin((caddr_t) uap->arg1, (caddr_t) &nvpx, sizeof(nvpx)))
			u.u_error = EFAULT;
		else if ((unsigned int) nvpx.addr < ONVRAM
		    || (unsigned int) nvpx.addr > ONVRAM + NVRSIZ
		    || (unsigned int) nvpx.addr + nvpx.cnt < ONVRAM
		    || (unsigned int) nvpx.addr + nvpx.cnt > ONVRAM + NVRSIZ)
			u.u_error = EINVAL;
		else if ( ! userdma(nvpx.data, nvpx.cnt, B_WRITE) )
			u.u_error = EFAULT;
		else {
			wnvram(nvpx.data, nvpx.addr, nvpx.cnt);
			undma(nvpx.data, nvpx.cnt, B_WRITE);
		}
		break;


	case RNVR:	/* read NVRAM */
		if (!suser())
			break;
		if (copyin((caddr_t) uap->arg1, (caddr_t) &nvpx, sizeof(nvpx)))
			u.u_error = EFAULT;
		else if ((unsigned int) nvpx.addr < ONVRAM
		    || (unsigned int) nvpx.addr > ONVRAM + NVRSIZ
		    || (unsigned int) nvpx.addr + nvpx.cnt < ONVRAM
		    || (unsigned int) nvpx.addr + nvpx.cnt > ONVRAM + NVRSIZ)
			u.u_error = EINVAL;
		else if ( ! userdma(nvpx.data, nvpx.cnt, B_READ) )
			u.u_error = EFAULT;
		else {
			rnvram(nvpx.addr, nvpx.data, nvpx.cnt);
			undma(nvpx.data, nvpx.cnt, B_READ);
		}
		break;




	/*
	 * Double-map data segment feature required for basic(1)
	 *
	 * sys3b(S3BDMM, flag)
	 */
	case S3BDMM:
		if (uap->arg1) {
			/*
			 * Non-zero flag; enable the double mapping
			 */

			register int	addr;

			if (!u.u_dmm) {
				addr = (int) dmmatt(&u);
				if (addr) {
					u.u_dmm = 1;
					u.u_rval1 = addr;
				}
			} else {
				u.u_rval1 = DMMADDR;
			}
		} else {
			/*
			 * Zero flag; disable the double mapping
			 */

			if (u.u_dmm) {
				dmmdet(&u);
				u.u_dmm = 0;
			}
		}
		break;

	case CHKSER:	/* Check soft serial number */
		if (!suser())
			break;
		if (chksrl(uap->arg1) == FAIL) {
			u.u_error = EINVAL;
		}
		break;


	/*	Read the EDT (Equiped Device Table).
	**		sys3b(S3BEDT, buffer, size_bytes);
	*/

	case S3BEDT:
	{
		register int i, size, sub_num, edt_size, *buffer;
		struct edt *edtptr;
		int edt_num;

		buffer = (int *)uap->arg1;
		size = uap->arg2;

		sub_num = 0;
		for (i = 0; i < VNUM_EDT; i++) {
			edtptr = V_EDTP(i);
			sub_num = sub_num + edtptr->n_subdev;
		}
		edt_num = (VNUM_EDT << 16 | sub_num);
		if (size < sizeof(int)) {
			u.u_error = EFAULT;
			break;
		}
		if (copyout(&edt_num, buffer, sizeof(int)) == -1) {
			u.u_error = EFAULT;
			break;
		}
		if (size == sizeof(int))
			break;
		else	{
			buffer++;
			size -= sizeof(int);
			edt_size = VNUM_EDT * sizeof(struct edt) +
					sub_num * sizeof(struct subdevice);
			if (size < edt_size) {
				u.u_error = EFAULT;
				break;
			}
			if (copyout(VP_EDT, buffer, edt_size) == -1) {
				u.u_error = EFAULT;
				break;
			}
		}
		break;
	}

	/*  return the size of memory */
	case S3BMEM:
	{
		u.u_rval1 = VMEMSIZE;
		break;
	}

	/*	This function is just here for compatibility.
	**	It is really just a part of S3BSWPI.  This
	**	new function should be used for all new
	**	applications.
	**		sys3b(S3BSWAP, path, swplo, nblocks);
	*/

	case S3BSWAP:
	{
		swpi_t	swpbuf;

		swpbuf.si_cmd   = SI_ADD;
		swpbuf.si_buf   = (char *)uap->arg1;
		swpbuf.si_swplo = uap->arg2;
		swpbuf.si_nblks = uap->arg3;

		swapfunc(&swpbuf);
		break;
	}

	/*	General interface for adding, deleting, or
	**	finding out about swap files.  See swap.h
	**	for a description of the argument.
	**		sys3b(S3BSWPI, arg_ptr);
	**/

	case S3BSWPI:
	{

		swpi_t	swpbuf;

		if (copyin(uap->arg1, &swpbuf, sizeof(swpi_t)) < 0) {
			u.u_error = EFAULT;
			return;
		}
		swapfunc(&swpbuf);
		break;
	}

	/*	Transfer to firmware for debugging.  Breakpoints
	**	may be set there and then we can return to run a
	**	test.
	*/

	case S3BTODEMON : {
		if (!suser())
			break;
		if (!call_demon())
			u.u_error = EINVAL;
		break;
	}

	/*	Modify the control character which gets
	**	into or out of demon.
	*/

	case S3BCCDEMON :
		if (!suser())
			break;
		if (uap->arg1 == 0) {
			u.u_rval1 = Demon_cc;
		} else {
			Demon_cc = uap->arg2;
		}
		break;

	/*	Turn the cache on or off on a 32100.
	*/

	case S3BCACHE :
		if (!suser())
			break;
		
		if (uap->arg1) {
			if (!is32b()) {
				u.u_error = EINVAL;
				break;
			}
			cache_on();
		} else {
			cache_off();
		}
		break;

	/*	Delete memory from the available list.  Forces
	**	tight memory situation for load testing.
	*/

	case S3BDELMEM : {
		register pfd_t	*pfd;
		register int	ii;

		/*	Check for first time here.  If so,
		**	initialize deleted memory list to
		**	empty.
		*/

		if (Delmem.pf_next == NULL) {
			Delmem.pf_next = &Delmem;
			Delmem.pf_prev = &Delmem;
		}

		if (!suser())
			break;
		
		ii = uap->arg1;
		memlock();
		if (freemem < ii  ||
		   availrmem - ii < tune.t_minarmem  ||
		   availsmem - ii < tune.t_minasmem) {
			memunlock();
			u.u_error = EINVAL;
			break;
		}

		freemem -= ii;
		maxmem -= ii;
		availrmem -= ii;
		availsmem -= ii;
		Delmem_cnt += ii;

		while (ii--) {
			pfd = phead.pf_next;
			ASSERT(pfd != &phead);
			ASSERT(pfd->pf_flags & P_QUEUE);
			ASSERT(pfd->pf_use == 0);
			pfd->pf_next->pf_prev = &phead;
			phead.pf_next = pfd->pf_next;
			pfd->pf_flags &= ~P_QUEUE;
			if (pfd->pf_flags & P_HASH)
				if (!premove(pfd))
					cmn_err(CE_PANIC,
					  "sys3b - DELMEM premove failed");
			pfd->pf_next = Delmem.pf_next;
			pfd->pf_prev = &Delmem;
			pfd->pf_next->pf_prev = pfd;
			Delmem.pf_next = pfd;
		}

		memunlock();
		break;
	}

	/*	Add back previously deleted memory.
	*/

	case S3BADDMEM : {
		register pfd_t	*pfd;
		register int	ii;

		if (!suser())
			break;

		ii = uap->arg1;
		memlock();
		if (ii > Delmem_cnt) {
			memunlock();
			u.u_error = EINVAL;
			break;
		}

		Delmem_cnt -= ii;
		freemem += ii;
		maxmem += ii;
		availrmem += ii;
		availsmem += ii;

		while (ii--) {
			pfd = Delmem.pf_next;
			Delmem.pf_next = pfd->pf_next;
			pfd->pf_next->pf_prev = &Delmem;
			pfd->pf_prev = phead.pf_prev;
			pfd->pf_next = &phead;
			phead.pf_prev->pf_next = pfd;
			phead.pf_prev = pfd;
			pfd->pf_flags |= P_QUEUE;
		}
		memunlock();
		break;
	}

	/*	Tell a user whether we have a mau or not.
	*/

	case S3BFPHW:
		if (uap->arg1 % 4 != 0) {	/* arg is addr and must */
			u.u_error = EFAULT;	/* be on word boundary */
			break;
		}
		/* Return mau_present flag */
		if (suword(uap->arg1, mau_present) == -1)
			u.u_error = EFAULT;
		break;

	default:
		u.u_error = EINVAL;
	}
}
/*
 * greenled()
 *
 * Flash the green "power" LED at a given rate. Specify
 * zero to turn the LED steadily on.
 */
greenled(rate)
register int	rate;
{
	register int	s;
	static int	isoff;
	static int	tout;

	s = splhi();
	if (isoff) {
		IDUART->scc_ropbc = PWRLED;
		isoff = 0;
	} else if (rate) {
		IDUART->scc_sopbc = PWRLED;
		isoff = 1;
	}
	if (tout) {
		untimeout(tout);
		tout = 0;
	}
	if (rate) {
		tout = timeout(greenled, rate, rate);
	}
	splx(s);
}

/*	Sys3b function 3 - manipulate swap files.
 */

swapfunc(si)
register swpi_t	*si;
{
	register int		i;
	register inode_t	*ip;

	switch (si->si_cmd) {
		case SI_LIST:
			i = sizeof(swpt_t) * MSFILES;
			if (copyout(swaptab, si->si_buf, i) < 0)
				u.u_error = EFAULT;
			break;

		case SI_ADD:
		case SI_DEL:
			if (!suser())
				break;
			u.u_dirp = si->si_buf;
			if ((ip = namei(upath, 0)) == NULL)
				break;
			if (ip->i_ftype != IFBLK) {
				iput(ip);
				u.u_error = ENOTBLK;
				break;
			}

			if (si->si_cmd == SI_DEL)
				swapdel(ip->i_rdev, si->si_swplo);
			else
				swapadd(ip->i_rdev, si->si_swplo,
					si->si_nblks);
			iput(ip);
			break;
	}
}

/*
 * chnvram()
 *
 * Check non-volatile RAM using similar checksum technique as
 * with ROM. Calls the appropriate firmware function, optionally
 * saving a corrected checksum.
 */
chnvram(mode)
register char	mode;
{
	return (VCHKNVRAM(mode));
}

/*
 * rnvram()
 *
 * Read non-volatile RAM. Calls the appropriate firmware routine.
 */
rnvram(src, dst, count)
register caddr_t	src;
register caddr_t	dst;
register unsigned short	count;
{
	return (VRNVRAM(src, dst, count));
}

/*
 * wnvram()
 *
 * Write non-volatile RAM. Calls the appropriate firmware routine.
 */
wnvram(src, dst, count)
register caddr_t	src;
register caddr_t	dst;
register unsigned short count;
{
	return (VWNVRAM(src, dst, count));
}

/*
 * Double map data segment feature
 */


/*
 * double map the data segment for the current process; both u_segdata and
 * the MMU descriptors are modified.  If no error, return the address of
 * the origin of the double mapping.
 */

caddr_t
dmmatt(up)
register user_t	*up;
{
	register preg_t	*dprp;
	register preg_t	*dmmprp;

	if ((dprp = findpreg(up->u_procp, PT_DATA)) == NULL) {
		u.u_error = EINVAL;
		return(NULL);
	}
	reglock(dprp->p_reg);
	dmmprp = attachreg(dprp->p_reg, up, DMMADDR, PT_DMM, SEG_RO);
	regrele(dprp->p_reg);
	if (dmmprp == NULL)
		return(NULL);
	if (up == &u)
		loadmmu(u.u_procp, SCN2);
	return(dmmprp->p_regva);
}


/*
 * remove the data segment double mapping for the current process; both
 * u_segdata and the MMU descriptors are modified.
 */

dmmdet(up)
register user_t	*up;
{
	register preg_t		*prp;
	register reg_t		*rp;
	register inode_t	*ip;

	if (!up->u_dmm)
		return;
	prp = findpreg(up->u_procp, PT_DMM);
	if (prp == NULL)
		return;
	rp = prp->p_reg;
	if (ip = rp->r_iptr)
		plock(ip);
	reglock(rp);
	detachreg(prp, up);
	if (up == &u)
		loadmmu(u.u_procp, SCN2);
}


/*********************************************************************/
/*                                                                  */
/*    Function Name: chksrl                                         */
/*                                                                  */
/*           Author: J. D. Jennings                                 */
/*                                                                  */
/*          Purpose: Check the passed suspected system serial	    */
/*		     number against the actual serial number stored */
/*		     in PROM.					    */
/*                                                                  */
/*     Return Value: PASS or FAIL depending if a match was good.    */
/*                                                                  */
/* Globals Affected: None                                           */
/*                                                                  */
/********************************************************************/

chksrl(passer)

int passer;

{
	int promser;

			/* Compute unit's stored serial number */

	promser = (VSERNO->serial3 +
		  (VSERNO->serial2 << 8) +
		  (VSERNO->serial1 << 16) +
		  (VSERNO->serial0 << 24));

	if (promser != passer) {
			return(FAIL);
	}

	return(PASS);
}

/*
 * Machine dependent code to reboot
 */
mdboot(fcn, mdep)
{
	switch (fcn) {
	case AD_HALT:
		splhi();
		VRUNFLG = SAFE;
		((struct duart *)CONS)->ip_opcr=0x00;
		((struct duart *)CONS)->scc_sopbc=KILLPWR; /*set power off bit*/
		while (1);

	case AD_BOOT:
		splhi();
		VRUNFLG = SAFE;
		rtnfirm();

	case AD_IBOOT:
		splhi();
		VRUNFLG = REENTRY;
		rtnfirm();
	}
}

call_demon()
{
	extern struct gate_l2	gatex[];
	int			nrmx_XX();
	register sde_t		*sdeptr;

	if (VSERNO->serial0 <= 0xB || VSERNO->serial0 >= 0x20)
		return(0);
	if ((int)(VBASE->p_serno) != 0xfff0)
		return(0);
	sdeptr = (sde_t *)srama[0];

	sdeptr->seg_prot = KRWE | URE;
	*gateptbl0 &= ~PG_COPYW;
	flushmmu(0, 1);
	VDEMON();
	sdeptr->seg_prot = KRE | URE;
	*gateptbl0 |= PG_COPYW;
	flushmmu(0, 1);
	return(1);
}


/*	The following routines are used for turning the
**	cache on and off on the 32100.
*/

/*  The following code is used to implement common kernel  */

struct fake_gate_l2
	{
	long psw;
	long pc;
	};

struct fake_kpcb
	{
	long psw;
	long tmp0[2];
	long psw2;
	long tmp[16];
	};

/*	The following is the number of second level gate
**	entries.
*/

#define	GATE_ENTRIES	64+16+1+29

extern struct fake_gate_l2 gates[];
extern struct fake_kpcb *Xproc, *Ivect[], kpcb_pswtch;
extern struct fake_kpcb kpcb_syscall;
extern long		sendsig_psw;
extern long		p0init_psw;;
extern int		winublock();
extern int		winubunlock();

char u400;

cache_off()
{
	register struct fake_kpcb	*ptr;
	register struct fake_kpcb	**ptr2;
	register struct fake_gate_l2	*ptr1;
	register proc_t			*pp;
	register user_t			*up;
	sde_t				*sdeptr;
	int				oldpri;

	sdeptr = (sde_t *)srama[0];
	sdeptr->seg_prot = KRWE | URE;
	*gateptbl0 &= ~PG_COPYW;
	flushmmu(0, 1);
	oldpri = splhi();

	for (ptr2 = &Xproc; ptr2 < &Ivect[256]; ptr2++) {
		ptr = *ptr2;
		ptr->psw = ptr->psw | ICACHEPSW;
		ptr->psw2 = ptr->psw2 | ICACHEPSW;
	}

	for (ptr1 = gates; ptr1 < &gates[GATE_ENTRIES]; ptr1++) {
		ptr1->psw = ptr1->psw | ICACHEPSW;
	}

	kpcb_pswtch.psw |= ICACHEPSW;
	kpcb_pswtch.psw2 |= ICACHEPSW;
	kpcb_syscall.psw |= ICACHEPSW;
	kpcb_syscall.psw2 |= ICACHEPSW;
	sendsig_psw |= ICACHEPSW;
	p0init_psw |= ICACHEPSW;

	winublock();
	up = (user_t *)win_ublk;
	for (pp = proc  ;  pp < (proc_t *)v.ve_proc  ;  pp++) {
		if (pp->p_stat == 0)
			continue;
		kvtokstbl(up)->wd2.address = kvtophys(ubptbl(pp));
		flushmmu(up, USIZE);
		ptr = (struct fake_kpcb *)&up->u_ipcb;
		ptr->psw |= ICACHEPSW;
		ptr->psw2 |= ICACHEPSW;
		ptr = (struct fake_kpcb *)&up->u_kpcb;
		ptr->psw |= ICACHEPSW;
		ptr->psw2 |= ICACHEPSW;
	}
	winubunlock();

	u400 = 0;

	asm("  ORW2  &0x02800000,%psw");
	asm("  NOP		     ");
	asm("  NOP		     ");

	splx(oldpri);
	sdeptr->seg_prot = KRE | URE;
	*gateptbl0 |= PG_COPYW;
	flushmmu(0, 1);
}

cache_on()
{
	register struct fake_kpcb	*ptr;
	register struct fake_kpcb	**ptr2;
	register struct fake_gate_l2	*ptr1;
	register proc_t			*pp;
	register user_t			*up;
	sde_t				*sdeptr;
	int				oldpri;

	sdeptr = (sde_t *)srama[0];
	sdeptr->seg_prot = KRWE | URE;
	*gateptbl0 &= ~PG_COPYW;
	flushmmu(0, 1);
	oldpri = splhi();

	for (ptr2 = &Xproc; ptr2 < &Ivect[256]; ptr2++) {
		ptr = *ptr2;
		ptr->psw = ptr->psw & ~ICACHEPSW;
		ptr->psw2 = ptr->psw2 & ~ICACHEPSW;
	}

	for (ptr1 = gates; ptr1 < &gates[GATE_ENTRIES]; ptr1++) {
		ptr1->psw = ptr1->psw & ~ICACHEPSW;
	}

	kpcb_pswtch.psw &= ~ICACHEPSW;
	kpcb_pswtch.psw2 &= ~ICACHEPSW;
	kpcb_syscall.psw &= ~ICACHEPSW;
	kpcb_syscall.psw2 &= ~ICACHEPSW;
	sendsig_psw &= ~ICACHEPSW;
	p0init_psw &= ~ICACHEPSW;

	winublock();
	up = (user_t *)win_ublk;
	for (pp = proc  ;  pp < (proc_t *)v.ve_proc  ;  pp++) {
		if (pp->p_stat == 0)
			continue;
		kvtokstbl(up)->wd2.address = kvtophys(ubptbl(pp));
		flushmmu(up, USIZE);
		ptr = (struct fake_kpcb *)&up->u_ipcb;
		ptr->psw &= ~ICACHEPSW;
		ptr->psw2 &= ~ICACHEPSW;
		ptr = (struct fake_kpcb *)&up->u_kpcb;
		ptr->psw &= ~ICACHEPSW;
		ptr->psw2 &= ~ICACHEPSW;
	}
	winubunlock();

	u400 = 1;

	asm("  CFLUSH		      ");
	asm("  ANDW2  &0xfd7fffff,%psw");
	asm("  NOP		      ");
	asm("  NOP		      ");

	splx(oldpri);
	sdeptr->seg_prot = KRE | URE;
	*gateptbl0 |= PG_COPYW;
	flushmmu(0, 1);
}


/*
 *  Routine returns the soft serial number
 */

readsrl()
{
	register int	promser;

	promser = (VSERNO->serial3 +
		  (VSERNO->serial2 << 8) +
		  (VSERNO->serial1 << 16) +
		  (VSERNO->serial0 << 24));

	return(promser);
}

/*
 * Routine flags if this is a fast 32B
 */

fast32b()
{
	register int	serial_number;

	/*	Read serial number to determine whether a 400 or a 300.
	 *	A 3 or 9 in nibble 5 is a 400.  Everything else is a 300.
	 */

	serial_number = (readsrl() >> 20) & 0xf;
	if (serial_number == 0x9 || serial_number == 0x3)
		return(1);
	return(0);
}

#define	POD_VADDR	0xC0F00000
#define	POD_NSEGS	8

mappod(pp)
register proc_t	*pp;
{
	register int	ii;
	register sde_t	*sdp;
	sde_t		tmpsde;

	if(growsdt(pp, SCN3, snum(POD_VADDR) + POD_NSEGS, NOSLEEP) < 0){
		cmn_err(CE_NOTE, "!mappod - couldn't grow segment table.");
		return(-1);
	}

	tmpsde.seg_prot = SEG_RW;
	tmpsde.seg_len  = (NBPS >> 3) - 1;
	tmpsde.seg_flags = SDE_kflags;

	sdp = &((sde_t *)(pp->p_srama[SCN3 - SCN2]))[snum(POD_VADDR)];

	for(ii = 0  ;  ii < POD_NSEGS  ;  ii++){
		tmpsde.wd2.address = (uint)POD_VADDR + stob(ii);
		sdp[ii] = tmpsde;
	}

	loadmmu(pp, SCN3);
	return(0);
}
