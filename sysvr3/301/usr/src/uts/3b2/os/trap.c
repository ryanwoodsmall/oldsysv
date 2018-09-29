/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/trap.c	10.17"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/sbd.h"
#include "sys/csr.h"
#include "sys/sit.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/sysinfo.h"
#include "sys/edt.h"
#include "sys/utsname.h"
#include "sys/firmware.h"
#include "sys/cmn_err.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/mau.h"


extern int sbdrcsr;
extern int sbdwcsr;
extern int bootstate;

int	mau_present;	/* flag if mau is in system */

/* read-only table of WE32100 support processor opcodes
 * used to distinguish between the (anonymous) external memory fault
 * caused by "support processor not present" and all other faults.
 * the entry format is 1) indicates MAU instruction 2) indicates double/triple
 * write instruction
 */
#define MAU_SPECIAL 2	/* flags that a page fault on this instruction needs
			 * special processing to restart the MAU
			 */
char spopcode[ 256 ] = {
    0,0,1,2,0,0,1,2,0,0,0,0,0,0,0,0,	/* 0x00 - 0x0f */
    0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,	/* 0x10 - 0x1f */
    0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x20 - 0x2f */
    0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x30 - 0x3f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x40 - 0x4f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x50 - 0x5f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x60 - 0x6f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x70 - 0x7f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x80 - 0x8f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x90 - 0x9f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0xa0 - 0xaf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0xb0 - 0xbf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0xc0 - 0xcf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0xd0 - 0xdf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0xe0 - 0xef */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	/* 0xf0 - 0xff */

int	*save_r0ptr ;	/* Pansave uses this to find	*/
			/* the registers.		*/

/*
 *	#################################################
 *	#						#
 *	#		NOTICE!#
 *	#						#
 *	# Although similar to u_trap(), s_trap() knows	#
 *	# that signal processing is the only thing to do#
 *	#						#
 *	#################################################
 */

s_trap()
{
	extern int	systrap();
	extern int	krnl_isp[];
	time_t		syst;
	register proc_t	*pp;

	u.u_kpcb.ipcb.pc = systrap;	/* reset entry point */
	u.u_pcbp = (struct pcb *)&u.u_kpcb.psw;	/* running on kernel now */

	syst = u.u_stime;
	psig();
	pp = u.u_procp;
	curpri = pp->p_pri = calcppri(pp);
	if (u.u_prof.pr_scale)
		addupc((caddr_t)u.u_ar0[PC],
			&u.u_prof,
			(int)(u.u_stime - syst));

	if (runrun != 0) /* signal handling used entire timeslice!*/
		qswtch();

	/* Return through common interrupt return sequence */

	ASSERT(noilocks() == 0);
	trap_ret();
}

addupc_clk()
{
	extern int	systrap();
	register proc_t	*pp;

	u.u_kpcb.ipcb.pc = systrap;	/* reset entry point */
	u.u_pcbp = (struct pcb *)&u.u_kpcb.psw;	/* running on kernel now */

	addupc(u.u_pcb.pc, &u.u_prof, 1);
	pp = u.u_procp;
	curpri = pp->p_pri = calcppri(pp);
	if ( runrun != 0 ) 
		qswtch();

	trap_ret();
}


/*
 * Called from ttrap.s if a trap occurs while on the kernel stack.
 *
 */

k_trap(r0ptr)
register int	*r0ptr;
{
	register int		i;
	register int		caddrsave;
	register union {
		psw_t	cps;
		int	cint;
	}			ps;
	register int	rr8, rr7, rr6, rr5, rr4, rr3;

	/* save pointer to r0 for use by pansave() */

	save_r0ptr = r0ptr ;
	ps.cint = r0ptr[PS];

	/*	If we were moving data to or from a user's
	**	process space and we got a memory fault,
	**	it may be an invalid page.  If so, validate
	**	it.  If not, give an error on the system
	**	call.
	*/

	if (u.u_caddrflt &&  ps.cps.FT == ON_NORMAL && ps.cps.ISC == XMEMFLT) {

		/*	Try to correct the fault.
		*/

		caddrsave = u.u_caddrflt;
		u.u_caddrflt = 0;
		i = usrxmemflt(r0ptr[PC],ps);

		/*	If usrxmemflt returned a non-zero value,
		**	then the fault couldn't be corrected.
		**	Return to the error routine indicated
		**	by u.u_caddrflt.  Otherwise, just return
		**	to try the access again.
		*/

		u.u_caddrflt = caddrsave;
		if (i != 0)
			r0ptr[PC] = u.u_caddrflt;
		else
			u.u_pgproc = 0;
		save_r0ptr = NULL;

		return;
	}

	/*	Make sure the compiler saves all of the registers.
	*/

	rr8 = rr7 = rr6 = rr5 = rr4 = rr3 = 1;;

	cmn_err(CE_CONT, "TRAP\nproc = %x psw = %x\npc = %x",
		u.u_procp, ps.cint, r0ptr[PC]);



	krnlflt(ps.cps);
	cmn_err(CE_PANIC, "Krnlflt returned to k_trap.");
}

/*
 * Called from the trap handler when a trap occurs on user stack.
 */

/*
 *	#################################################
 *	#						#
 *	#		NOTICE!#
 *	#						#
 *	#	U_TRAP() is CALLPS'ed not CALLed.	#
 *	#	It must RETPS, not RETURN.		#
 *	#						#
 *	#################################################
 */
u_trap()
{
	extern int		systrap();
	extern int		krnl_isp[];
	register int		sig;
	register struct user	*uptr = &u;
	register time_t		syst;
	register struct proc	*pp;
	register union {
		psw_t	cps;
		int	cint;
	}			ps;
	extern int		nrmx_ilc;
	extern int		nrmx_ilc2;
	extern int		nrmx_iop;
	extern int		nrmx_iop2;

	/* finish storing the user state in u.u_pcb */

	uptr->u_pcb.regsave[K_R0] = uptr->u_r0tmp;
	uptr->u_pcb.regsave[K_PS] = *(--uptr->u_pcb.sp);
	uptr->u_pcb.regsave[K_PC] = *(--uptr->u_pcb.sp);

	/* reset the kpcb initial pc so syscalls whizz through */

	uptr->u_kpcb.ipcb.pc = systrap;

	/* get off the interrupt stack!*/

	asm("	SUBW2	&4,%isp");

	MONITOR('U', uptr->u_pcb.regsave[K_PC], uptr->u_pcb.regsave[K_PS],
		uptr->u_procp, 0);

	ps.cint = uptr->u_pcb.regsave[K_PS];
	syst = uptr->u_stime;
	sig = 0;

	if (ps.cps.FT == ON_NORMAL) {

		switch (ps.cps.ISC) {
			case IZDFLT:
			case IOVFLT:
				sig = SIGFPE;
				break;
			case BPTRAP:
			case TRCTRAP:
				ps.cps.TE = 0;
				uptr->u_pcb.regsave[K_PS] = ps.cint;
				sig = SIGTRAP;
				break;
			case ILLOPFLT:
			case RESOPFLT:
			case IVDESFLT:
			case RDTPFLT:
			case ILCFLT:
			case PRVOPFLT:
			case PRVREGFLT:
				sig = SIGILL;
				break;
			case GVFLT:
				sig = SIGSYS;
				break;
			case XMEMFLT:
				sig = usrxmemflt(uptr->u_pcb.pc,uptr->u_pcb.psw);
				break;
		}
	} else {
		/*	We must have gotten here for an
		**	invalid "gate" done by the user.
		*/

		sig = SIGKILL;
	}

	pp = uptr->u_procp;
	curpri = pp->p_pri = calcppri(pp);

	if (sig != 0)
		psignal(uptr->u_procp, sig);

	/*	We may be running on the user pcb if we are
	**	fielding an ilc or returning from a floating
	**	point routine called via interrupt.  In this
	**	case, the return address is in the u_pcb.
	**	If we context switch or try to call a user
	**	signal routine, we will clobber this return
	**	address.  Don't do that.
	*/

	if((uptr->u_pcb.regsave[K_PC] >= (int)&nrmx_ilc   &&
	    uptr->u_pcb.regsave[K_PC] <= (int)&nrmx_ilc2) ||
	   (uptr->u_pcb.regsave[K_PC] >= (int)&nrmx_iop   &&
	    uptr->u_pcb.regsave[K_PC] <= (int)&nrmx_iop2)  ) {
		ASSERT(noilocks() == 0);
		trap_ret();
	}

	if (runrun != 0)
		qswtch();
	if ((uptr->u_procp->p_flag & SPRSTOP)
	  || (uptr->u_procp->p_sig && issig())) {
		psig();
		curpri = pp->p_pri = calcppri(pp);
	}
	if (uptr->u_prof.pr_scale) {
		addupc((caddr_t)uptr->u_pcb.regsave[K_PC],
			&uptr->u_prof,
			(int)(uptr->u_stime - syst));
	}
	

	/* Return through common interrupt return sequence */

	ASSERT(noilocks() == 0);
	trap_ret();
}

/*
 * Called from the trap handler when a system call occurs.
 */

/*
 *	#################################################
 *	#						#
 *	#		NOTICE!#
 *	#						#
 *	#	SYSTRAP() is CALLPS'ed, not CALL'ed.	#
 *	#	It must RETPS, not RETURN.		#
 *	#						#
 *	#################################################
 *
 */

systrap()
{
	register uint		i;
	register uint		*ap;
	register struct user	*uptr = &u;
	register struct sysent	*callp;
	register struct proc	*pp;
	extern int		krnl_isp[];
	time_t			syst;
	short			pid;

	/* finish storing the user state into u.u_pcb */

	uptr->u_pcb.regsave[K_R0] = uptr->u_r0tmp;
	uptr->u_pcb.regsave[K_PS] = *(--uptr->u_pcb.sp);
	uptr->u_pcb.regsave[K_PC] = *(--uptr->u_pcb.sp);

	/*	The following line is only really needed
	**	for process 1.  We come in here on the
	**	user stack but the saved PSW has kernel
	**	mode.  This is necessary since we had to
	**	write the pcb to switch stacks.  See the
	**	code in misc.s/icode.
	*/

	((psw_t *)(&uptr->u_pcb.regsave[K_PS]))->CM = PS_USER;

	/* get off the interrupt stack */

	asm("	SUBW2	&4,%isp ");

	MONITOR('S',
		uptr->u_pcb.regsave[K_PC],
		uptr->u_pcb.regsave[K_PS],
		uptr->u_pcb.regsave[K_R0],
		uptr->u_pcb.regsave[K_R1]);

	syst = uptr->u_stime;
	pid = u.u_procp->p_pid;

	sysinfo.syscall++;
	uptr->u_error = 0;
	((psw_t *)(uptr->u_pcb.regsave))[K_PS].NZVC &= ~PS_C;
	ap = (uint *)uptr->u_pcb.regsave[K_AP];

	u.u_syscall = (uptr->u_pcb.regsave[K_R1]&0x7ff8) >> 3;
	callp = &sysent[u.u_syscall];
	for (i = 0; i < callp->sy_narg; i++) {
		uptr->u_arg[i] = fuword(ap++);
	}
	uptr->u_dirp = (caddr_t)uptr->u_arg[0];
	uptr->u_rval1 = 0;
	uptr->u_rval2 = uptr->u_pcb.regsave[K_R1];
	uptr->u_ap = uptr->u_arg;

	if ((bootstate || !callp->sy_setjmp) && setjmp(uptr->u_qsav)) {
		if (!(uptr->u_rflags & U_RSYS) && !uptr->u_error)
			uptr->u_error = EINTR;
	} else
		(*callp->sy_call)();

	uptr->u_rflags &= ~(U_RSYS | U_DOTDOT | U_LBIN);

	if (uptr->u_error) {
		uptr->u_pcb.regsave[K_R0] = uptr->u_error;
		uptr->u_error = 0;
		((psw_t *)(uptr->u_pcb.regsave))[K_PS].NZVC |= PS_C;
		if (++uptr->u_errcnt > 16) {
			uptr->u_errcnt = 0;
			runrun++;
		}
	} else {
		uptr->u_pcb.regsave[K_R0] = uptr->u_rval1;
		uptr->u_pcb.regsave[K_R1] = uptr->u_rval2;
	}

	MONITOR('R',
		uptr->u_pcb.regsave[K_PC],
		uptr->u_error,
		uptr->u_pcb.regsave[K_R0],
		uptr->u_pcb.regsave[K_R1]);


	pp = uptr->u_procp;
	if (runrun != 0) {
		curpri = pp->p_pri = calcppri(pp);
		qswtch();
	}


	if ((pp->p_flag & SPRSTOP) || (pp->p_sig && issig()))
		psig();
	curpri = pp->p_pri = calcppri(pp);

	/*	If pid != pp->p_pid, then we are the child
	**	returning from a fork system call.  In this
	**	case, ignore syst since our time was reset
	**	in fork.
	*/

	if (uptr->u_prof.pr_scale)
		addupc((caddr_t)uptr->u_ar0[PC], &uptr->u_prof, 
			pid == pp->p_pid ? (int)(uptr->u_stime - syst)
					 : (int)uptr->u_stime);

	/* Return through common interrupt return sequence */

	ASSERT(noilocks() == 0);
	trap_ret();
}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/* 
 * package not installed -- return ENOPKG error  (STUBS support)
 */
nopkg()
{
	u.u_error = ENOPKG;
}


/*
 * internal function call for uninstalled package -- panic system.  If the
 * system ever gets here, it means that an internal routine was called for
 * an optional package, and the OS is in trouble.  (STUBS support)
 */
noreach()
{
	cmn_err(CE_PANIC,"Call to internal routine of uninstalled package");
}


/*
 * stray interrupts enter here
 */

intnull()
{
}


/*	This routine is called for all level 15 interrupts
**	except clock interrupts.
*/

intsyserr ()
{
 	extern char pwrflag;

 	if (SBDSIT->count0 != SITINIT)  /*power down*/
 	{
 		pwrflag = 1;
 		((struct wcsr *)(&sbdwcsr))->s_pir9 = 0x1;  /* set PIR 9 */
 		SBDSIT->command=0x16;

		/*clear softpwr-bus timer bit */

		((struct wcsr *)(&sbdwcsr))->c_sanity = 0x00;
 		return;
 	}
	if ((Rcsr & CSRPARE) != 0)
	{
		Wcsr->c_parity = 0 ;
		cmn_err(CE_PANIC,"SYSTEM PARITY ERROR INTERRUPT") ;
	}
	if ((Rcsr & CSRTIMO) != 0)
	{
		Wcsr->c_sanity = 0 ;
		cmn_err(CE_PANIC,"SYSTEM BUS TIME OUT INTERRUPT") ;
	}
	return ;
}

/*
 * intspwr()
 *
 * Handle a softpower shutdown. Simply sends SIGPWR to init, allowing
 * it to gracefully stop the machine.
 */
intspwr()
{
	psignal(&proc[1], SIGPWR);
}

strcmp(s1, s2)
register char *s1, *s2;
{

	if (s1 == s2)
		return(0);
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return(*s1 - *--s2);
}



/*
 * process exception
 */

intpx(psw, pcbp)
psw_t psw;
struct pcb *pcbp;
{
	spl7();
	cmn_err(CE_PANIC,
		"process exception, proc = 0x%x, psw= 0x%x, pcbp = 0x%x.\n",
		u.u_procp, psw, pcbp) ;
}

/*	This routine is called for user stack exceptions.
*/

intsx(pcbp)
register struct pcb *pcbp;
{
	register int		sig;
	register struct proc	*pp;
	register int		rval;
	int			mau_faddr;
	int			mau_flg = 0;

	MONITOR('X', pcbp->pc, pcbp->psw, pcbp->sp, 0);

	/*	If it was a stack bounds fault, try to
	**	grow the stack.  If this succeeds, then
	**	just return.  Otherwise, the user's stack
	**	is blown.  Give him/her a SIGSEGV and be sure
	**	we don't try to call a user signal routine
	**	with the stack messed up.
	**
	**	If it was a stack fault, try to fix it up
	**	by loading the page or making it writable.
	**
	**	We don't expect to get an interrupt vector
	**	fetch fault.
	*/

	pp = u.u_procp;

	if (pcbp->psw.ISC == STKBOUND) {
		/* Check for the "lost SPOP write" */
		if (mau_present)
			mau_pfault(MAU_PROBESB, &mau_faddr, &mau_flg);
		if (grow(pcbp->sp)) {
			rval = 1;
			/*
			 * mau_flg == 1 only if mau_pfault was called and
			 * the current instruction is a non-restartable MAU
			 * operation -- complete the operation.
			 */
			if (mau_flg)
				mau_pfault(MAU_NOPROBE, &mau_faddr, &mau_flg);
		} else {
			u.u_signal[SIGSEGV-1] = SIG_DFL;
			psignal(pp, SIGSEGV);
			rval = 0;
		}
	} else if (pcbp->psw.ISC == STKFLT) {
		sig = usrxmemflt(pcbp->pc,pcbp->psw);
		if (sig) {
			u.u_signal[sig - 1] = SIG_DFL;
			psignal(pp, sig);
			rval = 0;
		} else {
			rval = 1;
		}
	} else {
		cmn_err(CE_PANIC,
			"Unexpected user stack fault, ISC = %x.",
			pcbp->psw.ISC);
	}

	/*
	 *	The following code is a hardware workaround to make
	 *	trace trap work.  If you have just executed an
	 *	instruction with the TE bit set in the psw,
	 *	you want to give a normal exception for trace.
	 *	However, if the current stack page is not valid
	 *	or you are at the top of the stack and have to
	 *	grow it, then you lose the normal exception and
	 *	have no idea what happened.  This code checks for
	 *	this.  The trace trap check is based on the
	 *	u_tracepc field of the u-block.  The code in ttrap.s
	 *	at trap_ret2 checks if the psw being restored has
	 *	the TE bit set.  If so, u_tracepc is set to the
	 *	pc being returned to.   If the TE bit is off, then
	 *	u_tracepc is cleared.  If we get here with u_tracepc
	 *	non-zero and not equal to the pc we got the stack
	 *	fault on, then we must have gotten the stack fault
	 *	trying to do the normal exception for the trace.
	 */

	if (rval) {
		if (u.u_tracepc && u.u_tracepc != (char *)pcbp->pc) {
			pcbp->psw.TE = 0;
			psignal(pp, SIGTRAP);
			rval = 0;
		}
	}

	curpri = pp->p_pri = calcppri(pp);
	return(rval);
}

/*	This routine is called for kernel stack exceptions.
*/

intsxk(pcbp)
register struct pcb	*pcbp;
{

	MONITOR('X', pcbp->pc, pcbp->psw, pcbp->sp, 0);

	spl7();
	cmn_err(CE_PANIC,"kernel process stack exception, ISC=%d.\n",
		pcbp->psw.ISC);
}

/*
 * Ignored system call
 */
nullsys()
{
}

stray(addr)
{
	cmn_err(CE_NOTE,"stray interrupt at %x\n", addr);
}

usrxmemflt(pc,ps)
register char	*pc;
psw_t	ps;
{
	register int	sig;
	register pde_t	*pde;
	register sde_t	*sde;
	register reg_t	*rp;
	FLTCR		faultcr;
	int		mau_faddr;
	int		mau_flg;
	int		faultadr;
	extern pde_t	*vatopde();
	extern sde_t	*vatosde();

	faultcr = *fltcr;
	faultadr = *fltar;
	*(int *)fltcr = 0 ;
	rp = findreg(u.u_procp, faultadr);
	if (rp)
		regrele(rp);

	if (mau_present && (ps.FT == ON_STACK))
	{
		mau_pfault(MAU_PROBESF, &mau_faddr, &mau_flg);
		/* check to see if mau_probe faulted in the page
		  * indicated by faultadr
		  */
		if ((faultadr & PG_ADDR) == (mau_faddr & PG_ADDR))
			return(0);
	}
	switch (faultcr.ftype)
	{

		case F_SDTLEN :
		case F_PDTLEN :
			if (rp == NULL  &&
			   faultadr >= (int)u.u_pcb.sub) {
				if (grow(faultadr)) {
					sde = vatosde(faultadr);
					pde = vatopde(faultadr, sde);
					if ((sig = vfault(faultadr, pde)) == 0 &&
					  spopcode[fubyte(pc)] == MAU_SPECIAL&&
					  faultcr.reqacc == AT_SPOPWRITE)
					    mau_pfault(MAU_NOPROBE, &faultadr,
						&mau_flg);
					break;
				}
			}
			sig = SIGSEGV;
			break;

		case F_P_N_P :
			if (rp == NULL  &&
			   faultadr >= (int)u.u_pcb.sub) {
				if (!grow(faultadr)) {
					sig = SIGSEGV;
					break;
				}
			}
			sde = vatosde(faultadr);
			pde = vatopde(faultadr, sde);
			if ((sig = vfault(faultadr, pde)) == 0 &&
			  spopcode[fubyte(pc)] == MAU_SPECIAL&&
			  faultcr.reqacc == AT_SPOPWRITE)
			    mau_pfault(MAU_NOPROBE, &faultadr, &mau_flg);
			break;

		case F_PWRITE :
			sde = vatosde(faultadr);
			pde = vatopde(faultadr, sde);
			if (!pg_isvalid(pde)) {
				sig = vfault(faultadr, pde);
				if (sig)
					break;
			}
			if ((sig = pfault(faultadr, pde)) == 0 &&
			  spopcode[fubyte(pc)] == MAU_SPECIAL&&
			  faultcr.reqacc == AT_SPOPWRITE)
			    mau_pfault(MAU_NOPROBE, &faultadr, &mau_flg);
			break;

		case F_ACCESS :
			sig = SIGBUS ;
			break ;

		case F_OFFSET :
		case F_ACC_OFF :
		case F_INVALID :
			sig = SIGSEGV ;
			break ;

		case F_MPROC :
		case F_RMUPDATE :
		case F_SEG_N_P :
		case F_OBJECT :
		case F_PDT_N_P :
		case F_INDIRECT :
		case F_D_P_HIT :
			sig = SIGKILL ;
			break ;

		default :
			/*	We can get here for 3 cases.
			**	An alignment error, a support
			**	processor instruction executed
			**	on a 32100 which does not have
			**	the support processor hardware
			**	or a gate instruction executed
			**	with one or two bad index values.
			**	Note that on a 32000, an spop
			**	instruction will generate an
			**	ILLOPFLT fault.
			*/

			if (Rcsr & CSRALGN) {
				sig = SIGEMT ;
				Wcsr->c_align = 0 ;
			} else if ((Rcsr & CSRPARE) != 0) {
				Wcsr->c_parity = 0 ;
				cmn_err(CE_PANIC,"SYSTEM PARITY ERROR INTERRUPT") ;
			} else if (spopcode[fubyte(pc)] != 0) {
				if (mau_present)
					sig = mau_fault(pc);
				else
					sig = SIGILL;
			} else {
				sig = SIGKILL ;
			}
			break ;
	}

	return(sig);
}

krnlflt(ps)
psw_t	ps;			/* previous PSW content */
{
	register int	i;
	static	char	*isc_msg[] = {
					"DIVIDE-BY-ZERO" ,
					"TRACE TRAP" ,
					"ILLEGAL OPCODE" ,
					"RESERVED OPCODE" ,
					"INVALID DESCRIPTOR" ,
					"EXTERNAL" ,
					"GATE VECTOR" ,
					"ILLEGAL LEVEL CHANGE" ,
					"RESERVED DATA TYPE" ,
					"INTEGER OVERFLOW" ,
					"PRIVILEGED OPCODE" ,
					"(UNKNOWN:11)" ,
					"(UNKNOWN:12)" ,
					"(UNKNOWN:13)" ,
					"BREAKPOINT TRAP" ,
					"PRIVILEGED REGISTER" ,
					} ;

	static	struct {
		int	errno ;
		char	*name ;
	}			mmu_err[] = {
					F_MPROC ,	"F_MPROC" ,
					F_RMUPDATE ,	"F_RMUPDATE" ,
					F_SDTLEN ,	"F_SDTLEN" ,
					F_PWRITE ,	"F_PWRITE" ,
					F_PDTLEN ,	"F_PDTLEN" ,
					F_INVALID ,	"F_INVALID" ,
					F_SEG_N_P ,	"F_SEG_N_P" ,
					F_OBJECT ,	"F_OBJECT" ,
					F_PDT_N_P ,	"F_PDT_N_P" ,
					F_P_N_P ,	"F_P_N_P" ,
					F_INDIRECT ,	"F_INDIRECT" ,
					F_ACCESS ,	"F_ACCESS" ,
					F_OFFSET ,	"F_OFFSET" ,
					F_ACC_OFF ,	"F_ACC_OFF" ,
					F_D_P_HIT ,	"F_D_P_HIT" ,
				} ;

	spl7() ;

	if ((Rcsr & CSRALGN) != 0) {
		Wcsr->c_align = 0 ;
		cmn_err(CE_PANIC,"KERNEL DATA ALIGNMENT ERROR") ;
	}

	if ((Rcsr & CSRPARE) != 0) {
		Wcsr->c_parity = 0 ;
		cmn_err(CE_PANIC,"SYSTEM PARITY ERROR INTERRUPT") ;
	}

	if ((Rcsr & CSRTIMO) != 0) {
		Wcsr->c_sanity = 0 ;
		cmn_err(CE_PANIC,"KERNEL BUS TIMEOUT") ;
	}

	if (ps.FT == ON_NORMAL)
		if (ps.ISC == XMEMFLT) {
			int	fltcr_type;

			fltcr_type = fltcr->ftype;
			*(int *)fltcr = 0;
			for (i=0 ;
			    i<sizeof(mmu_err)/sizeof(mmu_err[0]) ; i++)
				if (mmu_err[i].errno == fltcr_type)
					break ;
			if (mmu_err[i].errno == fltcr_type)
				cmn_err(CE_PANIC,
					"KERNEL MMU FAULT (%s)\n",
					mmu_err[i].name) ;
			else
				cmn_err(CE_PANIC,
					"KERNEL MMU FAULT (%d)\n",
					fltcr_type) ;
		} else
			cmn_err(CE_PANIC,
				"KERNEL MODE %s FAULT\n",isc_msg[ps.ISC]) ;
	else
		cmn_err(CE_PANIC,
			"KERNEL MODE FAULT, FT=%d, ISC=%d\n",ps.FT,ps.ISC) ;
}
