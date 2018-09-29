/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/prf.c	10.9"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/panregs.h"
#include "sys/sbd.h"
#include "sys/sit.h"
#include "sys/csr.h"
#include "sys/immu.h"
#include "sys/edt.h"
#include "sys/cmn_err.h"
#include "sys/nvram.h"
#include "sys/firmware.h"
#include "sys/tty.h"
#include "sys/reg.h"
#include "sys/inline.h"

# define VRNVRAM	rnvram
# define VWNVRAM	wnvram

/* general argument list for cmn_err and panic */

# define ARGS	a0,a1,a2,a3,a4,a5

/*	A delay is required when outputting many
**	lines to the console if it is a dmd 5620
**	terminal.  The following value has been
**	chosen empirically.  If your console is
**	a normal terminal, set the delay to 0.
**	Note that dmd_delay can be set to 0
**	on the running system with demon.
*/

#define	DMD_DELAY	0x1000

int	dmd_delay = DMD_DELAY;


/*	Save output in a buffer where we can look at it
**	with demon or crash.  If the message begins with
**	a '!', then only put it in the buffer, not out
**	to the console.
*/

extern	char	putbuf[];
extern	int	putbufsz;
int		putbufndx;
short		prt_where;

#define	output(c) \
	if (prt_where & PRW_BUF) { \
		putbuf[putbufndx++ % putbufsz] = c; \
	} \
	if (prt_where & PRW_CONS) \
		iuputchar(c);

#define	loutput(c, tp) \
	if (prt_where & PRW_BUF) { \
		putbuf[putbufndx++ % putbufsz] = c; \
	} \
	if (prt_where & PRW_CONS) \
		putc(c, &tp->t_outq);

char	*panicstr ;

extern int	sbdpit ;
extern int	sbdrcsr ;
extern char	sbdnvram ;
extern char	*mmusrama ;
extern char	*mmusramb ;
extern char	*mmufltar ;
extern char	*mmufltcr ;
extern int	*save_r0ptr ;
extern int	sdata[];
extern int	bssSIZE[];

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 */

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register int	c;
	register uint	*adx;
	register char	*s;
	register int	opri;

	opri = splhi();
	adx = &x1;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			splx(opri);
			wakeup(putbuf);
			return;
		}
		output(c);
	}
	c = *fmt++;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			output(c);
		}
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

printn(n, b)
long n;
register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		output('-');
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		output("0123456789ABCDEF"[d[i]]);
	}
}

/*
 * Panic is called on unresolvable fatal errors.
 */

panic(ARGS)
int	ARGS ;
{
	struct	xtra_nvr	nvram_copy ;
	struct	xtra_nvr	*p ;
	int			x;
	int			i ;

					/* save panic string (needed for  */
					/* routines elsewhere		  */
	panicstr = (char *)a0 ;
					/* stop interval timer */
	((struct sit *)(&sbdpit))->command = ITINIT ;
					/* get nvram contents */
	p = (struct xtra_nvr *)(&sbdnvram+XTRA_OFSET) ;
	VRNVRAM(p,&nvram_copy,sizeof(nvram_copy)) ;

					/* make room for new error in log */
	for (i=NERRLOG-1 ; i>=1 ; i--)
		nvram_copy.errlog[i] = nvram_copy.errlog[i-1] ;
					/* enter new error in log */
	nvram_copy.errlog[0].message = (char *)a0 ;
	nvram_copy.errlog[0].param1  = a1 ;
	nvram_copy.errlog[0].param2  = a2 ;
	nvram_copy.errlog[0].param3  = a3 ;
	nvram_copy.errlog[0].param4  = a4 ;
	nvram_copy.errlog[0].param5  = a5 ;
	nvram_copy.errlog[0].time    = time ;
					/* save system state */
	pansave(&nvram_copy.systate) ;
					/* update nvram */
	nvram_copy.nvsanity = ~NVSANITY ;
	VWNVRAM(&nvram_copy,p,sizeof(nvram_copy)) ;
					/* mark as sane */
	nvram_copy.nvsanity = NVSANITY ;
	VWNVRAM(&nvram_copy,&(p->nvsanity),sizeof(nvram_copy.nvsanity)) ;
					/* get execution level */
	x = splhi();
	splx(x);

					/* "sync" */
	if (((psw_t *)&x)->IPL == 0)
		update();
					/* check for optional reboot */
	mtcrchk();
					/* return to firmware */
	call_demon();
	rtnfirm();
}

/*
 * prdev prints a warning message.
 * dev is a block special device argument.
 */

prdev(str, dev)
char *str;
dev_t dev;
{
	register maj;

	maj = bmajor(dev);
	if (maj >= bdevcnt) {
		cmn_err(CE_WARN,"%s on bad dev %o(8)\n", str, dev);
		return;
	}
	(*bdevsw[maj].d_print)(dev, str);
}

/*
 * prcom prints a diagnostic from a device driver.
 * prt is device dependent print routine.
 */
prcom(prt, bp, er1, er2)
int (*prt)();
register struct buf *bp;
{
	(*prt)(bp->b_dev, "\nDevice error");
	cmn_err(CE_NOTE,"bn = %D er = %o,%o\n", bp->b_blkno, er1, er2);
}

r0()				/* returns value of r0 (!) */
{
}

# define REG(x,r)	asm(" MOVW r,%r0") ; x = r0() ;

pansave(p)
struct	systate	*p ;
{
	/* save_r0ptr is set in trap when trap is entered
	 * save_r0ptr points to R0 in the stack after the trap
	 * the format of the stack is in ttrap.s
	 * ofp gets the fp of the process that caused the trap
	 * lfp gets the last frame pointer (of pansave)
	 */
	
	p->csr = Rcsr ;
	if (save_r0ptr != NULL) {
		*(int *)&p->psw = save_r0ptr[PS] ;
		p->r3 = save_r0ptr[R3] ;
		p->r4 = save_r0ptr[R4] ;
		p->r5 = save_r0ptr[R5] ;
		p->r6 = save_r0ptr[R6] ;
		p->r7 = save_r0ptr[R7] ;
		p->r8 = save_r0ptr[R8] ;
		p->oap = save_r0ptr[AP] ;
		p->opc = save_r0ptr[PC] ;
		p->osp = save_r0ptr[SP] ;
		p->ofp = save_r0ptr[FP] ;
	}
	REG(p->lfp,%fp)
	REG(p->isp,%isp)
	REG(p->pcbp,%pcbp)

	p->mmufltcr = *fltcr ;
	p->mmufltar = *(long *)fltar ;
	p->mmusrama[0] = *(SRAMA *)(srama) ;
	p->mmusrama[1] = *(SRAMA *)(srama + 1) ;
	p->mmusrama[2] = *(SRAMA *)(srama + 2) ;
	p->mmusrama[3] = *(SRAMA *)(srama + 3) ;
	p->mmusramb[0] = *(SRAMB *)(sramb) ;
	p->mmusramb[1] = *(SRAMB *)(sramb + 1) ;
	p->mmusramb[2] = *(SRAMB *)(sramb + 2) ;
	p->mmusramb[3] = *(SRAMB *)(sramb + 3) ;
}


# define IUCONSOLE	0
# define SEC		300000	/* approx # loops per second */

int	panic_level = 0;

cmn_err(level, fmt, ARGS)
register int	level ;
char		*fmt;
int		ARGS ;
{
	register int	*ip;
	register int	i;
	register int	x;
	register int	r4save;
	register int	r3save;
	int		r0save;
	int		r1save;
	int		r2save;
	pcb_t		regsave_pcb;

	/*	Save the volatile registers as quickly as
	**	possible.  Sure hope the compiler sets up
	**	the stack frame in the obvious way.  This
	**	is used only for a panic but we must do
	**	the save now.
	*/

	asm(" MOVW %r0,0(%fp)");
	asm(" MOVW %r1,4(%fp)");
	asm(" MOVW %r2,8(%fp)");

	/*	Set up to print to putbuf, console, or both
	**	as indicated by the first character of the
	**	format.
	*/

	if (*fmt == '!') {
		prt_where = PRW_BUF;
		fmt++;
	} else if (*fmt == '^') {
		prt_where = PRW_CONS;
		fmt++;
	} else {
		prt_where = PRW_BUF | PRW_CONS;
	}

	switch (level) {
		case CE_CONT :
			errprintf(fmt, ARGS) ;
			break ;

		case CE_NOTE :
			errprintf("\nNOTICE: ") ;
			errprintf(fmt, ARGS) ;
			errprintf("\n") ;
			break ;

		case CE_WARN :
			errprintf("\nWARNING: ") ;
			errprintf(fmt, ARGS) ;
			errprintf("\n") ;
			break ;

		case CE_PANIC : {
			switch (panic_level) {
				case 0 : {
					x = splhi() ;

					/* drop DTR to kill layers */

					iumodem(IUCONSOLE,OFF) ;
					i = 2 * SEC ;
					while (--i)
						 ;
					/* raise DTR before sending */
					/* message		    */

					iumodem(IUCONSOLE,ON) ;
					i = 2 * SEC ;
					while (--i)
						;

					prt_where = PRW_CONS | PRW_BUF;
					panic_level = 1 ;
					printf("\nPANIC: ") ;
					printf(fmt, ARGS) ;
					printf("\n") ;
					splx(x) ;

					/* If the registers were not saved,
					** save them now.
					*/

					if (save_r0ptr == NULL) {

						save_r0ptr =
						  &regsave_pcb.regsave[K_R0];

						save_r0ptr[R0] = r0save;
						save_r0ptr[R1] = r1save;
						save_r0ptr[R2] = r2save;

						asm(" MOVAW -9*4(%fp),%r7");

						save_r0ptr[PC] = ip[0];
						save_r0ptr[AP] = ip[1];
						save_r0ptr[FP] = ip[2];
						save_r0ptr[R3] = ip[3];
						save_r0ptr[R4] = ip[4];
						save_r0ptr[R5] = ip[5];
						save_r0ptr[R6] = ip[6];
						save_r0ptr[R7] = ip[7];
						save_r0ptr[R8] = ip[8];
					} else {
						ip = save_r0ptr;
						save_r0ptr =
						  &regsave_pcb.regsave[K_R0];

						save_r0ptr[PC] = ip[PC];
						save_r0ptr[PS] = ip[PS];
						save_r0ptr[AP] = ip[0];
						save_r0ptr[FP] = ip[1];
						save_r0ptr[R3] = ip[2];
						save_r0ptr[R4] = ip[3];
						save_r0ptr[R5] = ip[4];
						save_r0ptr[R6] = ip[5];
						save_r0ptr[R7] = ip[6];
						save_r0ptr[R8] = ip[7];
					}
					panic(fmt, ARGS) ;
				}

				case 1 :
					panic_level = 2 ;
					prt_where = PRW_CONS | PRW_BUF;
					printf("\nDOUBLE PANIC: ") ;
					printf(fmt, ARGS) ;
					printf("\n") ;
					call_demon();
					rtnfirm() ;

				default :
					panic_level = 3 ;
					rtnfirm() ;
			}
		}

		default :
			cmn_err(CE_PANIC,
			  "unknown level in cmn_err (level=%d, msg=\"%s\")",
			  level, fmt, ARGS) ;
	}
}


printputbuf()
{
	register int		pbi;
	register int		cc;
	register int		lim;
	register struct tty	*tp;
	register int		opl;
	int			delay;
	struct tty		*errlayer();

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	opl = splhi();

	if (conlayers() == 1) {
		if (cfreelist.c_next == NULL) {
			splx(opl);
			return;
		}
		tp = errlayer();  /* get tty pointer to error layer */
	} else {
		tp = NULL;
	}

	pbi = putbufndx % putbufsz;
	lim = putbufsz;

	while (1) {
		if (pbi < lim  &&  (cc = putbuf[pbi++])) {
			if (tp)
				putc(cc, tp->t_outq);
			else
				iuputchar(cc);
			if (cc == '\n'  &&  dmd_delay) {
				delay = dmd_delay;
				while (delay--) ;
			}
		} else {
			if (lim == putbufndx % putbufsz) {
				break;
			} else {
				lim = putbufndx % putbufsz;
				pbi = 0;
			}
		}
	}

	if (tp)
		xtvtproc(tp, T_OUTPUT);	/* Output buffer. */
	splx(opl);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

printnvram()
{
	struct	xtra_nvr	nvram_copy ;
	struct	xtra_nvr	*p ;
	register int		oldpri;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	oldpri = splhi();
	p = (struct xtra_nvr *)(&sbdnvram+XTRA_OFSET) ;
	VRNVRAM(p,&nvram_copy,sizeof(nvram_copy)) ;
	dumpnvram(&nvram_copy) ;
	splx(oldpri);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

dumpnvram(p)
struct	xtra_nvr	*p ;
{
	int	i ;
	int	delay;
	static	char	*csr_names[] =
	{
		"i/o fault" ,
		"dma" ,
		"disk" ,
		"uart" ,
		"pir9" ,
		"pir8" ,
		"clock" ,
		"inhibit fault" ,
		"inhibit time" ,
		"unassigned" ,
		"floppy" ,
		"led" ,
		"alignment" ,
		"req reset" ,
		"parity" ,
		"bus timeout" ,
	} ;

	prt_where = PRW_CONS;
	printf("nvram status:\t") ;
	switch (p->nvsanity)
	{
		case NVSANITY :
			printf("sane") ;
			break ;

		case ~NVSANITY :
			printf("incompletely updated") ;
			break ;

		default :
			printf("invalid") ;
			break ;
	}
	printf("\n\n") ;
	printf("csr:\t%x\t",p->systate.csr) ;
	for (i=15 ; i>=0 ; i--)
		if ((p->systate.csr&(1<<i)) != 0)
			printf("%s, ",csr_names[i]) ;
	printf("\n") ;
	printf("\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
 	printf("psw:\tCFD QIE CD OE NZVC TE IPL CM PM R I ISC TM FT\n") ;
	printf
	(
		"(hex)\t  %x   %x  %x  %x  %x    %x  %x   %x  %x  %x %x %x   %x  %x\n",
		p->systate.psw.CSH_F_D,
		p->systate.psw.QIE,
		p->systate.psw.CSH_D,
		p->systate.psw.OE,
		p->systate.psw.NZVC,
		p->systate.psw.TE,
		p->systate.psw.IPL,
		p->systate.psw.CM,
		p->systate.psw.PM,
		p->systate.psw.R,
		p->systate.psw.I,
		p->systate.psw.ISC,
		p->systate.psw.TM,
		p->systate.psw.FT
	) ;
	printf("\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
	printf("r3:\t%x\n",p->systate.r3) ;
	printf("r4:\t%x\n",p->systate.r4) ;
	printf("r5:\t%x\n",p->systate.r5) ;
	printf("r6:\t%x\n",p->systate.r6) ;
	printf("r7:\t%x\n",p->systate.r7) ;
	printf("r8:\t%x\n",p->systate.r8) ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
	printf("oap:\t%x\n",p->systate.oap) ;
	printf("opc:\t%x\n",p->systate.opc) ;
	printf("osp:\t%x\n",p->systate.osp) ;
	printf("ofp:\t%x\n",p->systate.ofp) ;
	printf("isp:\t%x\n",p->systate.isp) ;
	printf("pcbp:\t%x\n",p->systate.pcbp) ;
	printf("\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
	printf("fltcr:\treqacc\txlevel\tftype\n") ;
	printf("\t%d\t%d\t%d\n", p->systate.mmufltcr.reqacc,
		p->systate.mmufltcr.xlevel,
		p->systate.mmufltcr.ftype) ;
	printf("fltar:\t0x%x\n",p->systate.mmufltar) ;
	printf("\n") ;
	printf("\tsrama\tsramb\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
	for (i=0 ; i<4 ; i++)
		printf("[%d]\t0x%x\t0x%x\n",
			i, p->systate.mmusrama[i],
			p->systate.mmusramb[i].SDTlen) ;
	printf("\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
	printf("time\tmessage\n") ;
	for (i=0 ; i<NERRLOG ; i++)
	{
		printf("\n") ;
		printf("-------------------------------------------\n") ;
		printf("%d\n",p->errlog[i].time) ;
		if ((unsigned)sdata <= (unsigned)p->errlog[i].message &&
		    (unsigned)p->errlog[i].message < (unsigned)sdata + (unsigned)bssSIZE)
			printf(p->errlog[i].message,
				p->errlog[i].param1,
				p->errlog[i].param2,
				p->errlog[i].param3,
				p->errlog[i].param4,
				p->errlog[i].param5) ;
		else
			printf("(0x%x, 0x%x, 0x%x)",
				p->errlog[i].message,
				p->errlog[i].param1,
				p->errlog[i].param2,
				p->errlog[i].param3,
				p->errlog[i].param4,
				p->errlog[i].param5) ;
		if (dmd_delay) {
			delay = dmd_delay;
			while (delay--) ;
		}
	}
	printf("\n") ;
	if (dmd_delay) {
		delay = dmd_delay;
		while (delay--) ;
	}
}

lprintf(fmt, x1)
register char	*fmt;
unsigned	x1;
{
	register		c;
	register uint		*adx;
	char			*s;
	register struct tty	*tp;
	register int		sr;	/* saved interrupt level */
	struct tty		*errlayer();

	tp = errlayer();	/* get tty pointer to error layer */
	sr = splhi();		/* ??? */
	if (cfreelist.c_next == NULL) { /* anywhere to buffer output? */
		splx(sr);		/* back to where we were */
		return;			/* nope, just return	*/
	}
	adx = &x1;
loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			xtvtproc(tp, T_OUTPUT);	/* ??? */
			splx(sr);		/* back to where we were */
			return;
		}
		loutput(c, tp);
	}
	c = *fmt++;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		lprintn((long)*adx, c=='o'? 8: (c=='x'? 16:10), tp);
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			loutput(c, tp);
		}
	} else if (c == 'D') {
		lprintn(*(long *)adx, 10, tp);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

lprintn(n, b, tp)
long n;
register b;
register struct tty *tp;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		loutput('-', tp);
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		loutput("0123456789ABCDEF"[d[i]], tp);
	}
}

errprintf(fmt, ARGS)
{
	if (conlayers() == 1)
		lprintf(fmt, ARGS) ;
	else
		printf(fmt, ARGS) ;
}

/*	The following is an interface routine for the drivers.
*/

dri_printf(ARGS)
{
	cmn_err(CE_CONT, ARGS);
}

conlayers()		/* is the console running layers?? */
{
	struct tty	*tp;
	extern int	xtin();

	/* console has major dev. entry of 0 */
	/* get pointer to console tty structure */

	tp = cdevsw[0].d_ttys;

	/* use console line discipline and linesw */
	/* to really determine if layers is running */

	if (linesw[tp->t_line].l_input == xtin)
		return(1);	/* true, layers is running */
	else
		return(0);	/* false, layers is not running */
}

/*	Called by the ASSERT macro in debug.h when an
**	assertion fails.
*/

assfail(a, f, l)
register char *a, *f;
{
	/*	Save all registers for the dump since crash isn't
	 *	very smart at the moment.
	 */
	
	register int	r6, r5, r4, r3;

	cmn_err(CE_PANIC, "assertion failed: %s, file: %s, line: %d",
		a, f, l);
}


nomemmsg(func, count, contflg, lockflg)
register char	*func;
register int	count;
register int	contflg;
register int	lockflg;
{
	cmn_err(CE_NOTE,
		"%s - Insufficient memory to%s %d%s page%s - %s",
		func, (lockflg ? " lock" : " allocate"),
		count, (contflg ? " contiguous" : ""),
		(count == 1 ? "" : "s"),
		"system call failed");
}
