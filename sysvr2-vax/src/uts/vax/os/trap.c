/* @(#)trap.c	6.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#include "sys/seg.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/mtpr.h"

#include "sys/debug.h"

#define	USER	040		/* user-mode flag added to type */
#define	NSYSENT	64

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(sp, type, code, pc, ps)
{
	register i;
	time_t syst;

	syst = u.u_stime;
	u.u_ar0 = (int *)(0x80000000 - 18*4);
	if (USERMODE(ps))
		type |= USER;
	switch (type) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	case PROTFLT:
	case SEGFLT:
#ifdef OSDEBUG
		printf("PROT or TRANS fault: va=%x\n\t", code);
		if (i = (int)mapa(code))
			printf("pte = %x\n", *(int *)i);
		else
			printf("no pte\n");
		printf("\n");
#endif

	default:
		panicstr = "trap";	/* fake it for printfs */
		printf("user = ");
		printf("%x ", u.u_procp->p_addr);
		printf("pid = %d\t", u.u_procp->p_pid);
		printf("uarea = %x\n", &u);
		printf("ps = %x\n", ps);
		printf("pc = %x\n", pc);
		printf("trap type %x\n", type);
		printf("code = %x\n", code);
		panic(NULL);

	case PROTFLT + USER:	/* protection fault */
		{
			pte_t *pt;

			pt = (pte_t *)mapa(code);
			if (pt == NULL)
				i = SIGSEGV;
			else
				i = (pt->pgm.pg_v) ? SIGBUS : SIGSEGV;
		}
		goto growstack;

	case PRIVFLT + USER:	/* privileged instruction fault */
	case RSADFLT + USER:	/* reserved addressing fault */
	case RSOPFLT + USER:	/* reserved operand fault */
		i = SIGILL;
		break;

	case PGRDFLT + USER:	/* read error during page fault */
		i = SIGKILL;
		printf("pid %d killed due to page fault read error\n", u.u_procp->p_pid);
		break;

	case SYSCALL + USER:	/* sys call */
		panic("syscall");

	calcpri:
	{
		register struct proc *pp;

		pp = u.u_procp;
		pp->p_pri = (pp->p_cpu>>1) + PUSER + pp->p_nice - NZERO;
		curpri = pp->p_pri;
		if (runrun == 0)
			goto out;
	}

	case RESCHED + USER:	/* Allow process switch */
		qswtch();
		goto out;

	case ARTHTRP + USER:
		i = SIGFPE;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 */
	case SEGFLT + USER:	/* segmentation exception */
		i = SIGSEGV;
	growstack:
		if (grow(u.u_ar0[SP]) || grow(code))
			goto calcpri;
		break;

	case BPTFLT + USER:	/* bpt instruction fault */
	case TRCTRAP + USER:	/* trace trap */
		ps &= ~PS_T;	/* turn off trace bit */
		i = SIGTRAP;
		break;

	case XFCFLT + USER:	/* xfc instruction fault */
		i = SIGEMT;
		break;

	case CMPTFLT + USER:	/* compatibility mode fault */
				/* so far, just send a SIGILL signal */
		i = SIGILL;
		break;

	case ASTTRAP + USER:	/* user level profiling */
		addupc((caddr_t) pc, &u.u_prof, 1);
		return;
	}
	psignal(u.u_procp, i);

out:
	if (u.u_procp->p_sig && issig())
		psig(code);
}

stray(addr)
{
	logstray(addr);
	printf("stray interrupt at %x\n", addr);
}
