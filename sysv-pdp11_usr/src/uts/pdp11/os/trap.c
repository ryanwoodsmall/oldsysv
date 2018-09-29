/* @(#)trap.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#include "sys/seg.h"
#include "sys/sysinfo.h"

#define	SETD	0170011		/* SETD instruction */
#define	SYS	0104400		/* sys (trap) instruction */
#define	USER	040		/* user-mode flag added to type */
#define	NSYSENT	64
#define	MEMORY	((physadr)0177740) /* 11/70 "memory" subsystem */

/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
char	regloc[] =
{
	R0, R1, R2, R3, R4, R5, R6, R7, PS
};

/*
 * Called from trap.s when a processor trap occurs.
 * The arguments are the words saved on the system stack
 * by the hardware and software during the trap processing.
 * Their order is dictated by the hardware and the details
 * of C's calling sequence. They are peculiar in that
 * this call is not 'by value' and changed user registers
 * get copied back on return.
 * type is the kind of trap that occurred.
 */
trap(type, sp, r1, nps, r0, pc, ps)
int *pc;
dev_t type;
{
	register i;
	time_t syst;

	syst = u.u_stime;
	u.u_fpsaved = 0;
	u.u_ar0 = &r0;
	if (USERMODE(ps))
		type |= USER;
	switch(minor(type)) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	default:
		panicstr = "trap";	/* fake it for printfs */
		if (cputype != 40)
			printf("ka6 = %o\n", ka6->r[0]);
		printf("aps = %o\n", &ps);
		printf("pc = %o ps = %o\n", pc, ps);
		printf("trap type %o\n", type);
		panic(NULL);

	case RSADFLT + USER:	/* bus error */
		i = SIGBUS;
		break;

	/*
	 * If illegal instructions are not
	 * being caught and the offending instruction
	 * is a SETD, the trap is ignored.
	 * This is because C produces a SETD at
	 * the beginning of every program which
	 * will trap on CPUs without 11/45 FPU.
	 */
	case PRIVFLT + USER:	/* illegal instruction */
		if(fuiword(pc-1) == SETD && u.u_signal[SIGILL-1] == 0)
			goto out;
		i = SIGILL;
		break;

	case SYSCALL + USER:	/* sys call */
	{
		register *a;
		register struct sysent *callp;
		int (*fetch)();

		sysinfo.syscall++;
		u.u_error = 0;
		ps &= ~PS_C;
		a = pc;
		i = fuiword(a-1)&0377;
		if(i >= NSYSENT)
			callp = &sysent[0];
		else if (i==0) {	/* indirect */
			a = (int *)fuiword((caddr_t)(a));
			pc++;
			i = fuword((caddr_t)a);
			a++;
			if ((i & ~0377) != SYS)
				i = 0;	/* illegal */
			i &= 0377;
			if (i >= NSYSENT)
				i = 0;
			callp = &sysent[i];
			fetch = fuword;
		} else {
			callp = &sysent[i];
			pc += callp->sy_narg - callp->sy_nrarg;
			fetch = fuiword;
		}
		for (i=0; i<callp->sy_nrarg; i++)
			u.u_arg[i] = u.u_ar0[regloc[i]];
		for(; i<callp->sy_narg; i++)
			u.u_arg[i] = (*fetch)((caddr_t)a++);
		u.u_dirp = (caddr_t)u.u_arg[0];
		u.u_rval1 = u.u_ar0[R0];
		u.u_rval2 = u.u_ar0[R1];
		u.u_ap = u.u_arg;
		if (setjmp(u.u_qsav)) {
			if (u.u_error==0)
				u.u_error = EINTR;
		} else {
			(*callp->sy_call)();
		}
		if (u.u_error) {
			u.u_ar0[R0] = u.u_error;
			ps |= PS_C;	/* carry bit */
			if (++u.u_errcnt > 16) {
				u.u_errcnt = 0;
				runrun++;
			}
		} else {
			u.u_ar0[R0] = u.u_rval1;
			u.u_ar0[R1] = u.u_rval2;
		}
	}
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

	/*
	 * Since the floating exception is an
	 * imprecise trap, a user generated
	 * trap may actually come from kernel
	 * mode. In this case, a signal is sent
	 * to the current process to be picked
	 * up later.
	 */
	case ARTHTRP:	/* floating exception */
		psignal(u.u_procp, SIGFPE);
		return;

	case ARTHTRP + USER:
		i = SIGFPE;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 * This relies on the ability of the hardware
	 * to restart a half executed instruction.
	 * On the 11/40 this is not the case and
	 * the routine backup.s may fail.
	 * The classic example is on the instruction
	 *	cmp	-(sp),-(sp)
	 */
	case SEGFLT + USER:	/* segmentation exception */
	{
	int	osp;

		osp = sp;
		if(backup(u.u_ar0) == 0)
			if(grow(osp))
				goto calcpri;
		i = SIGSEGV;
		break;
	}

	case BPTFLT + USER:	/* bpt or trace */
		i = SIGTRAP;
		break;

	case IOTFLT + USER:	/* iot */
		i = SIGIOT;
		break;

	case EMTFLT + USER:	/* emt */
		i = SIGEMT;
		break;

	/*
	 * The code here is a half-hearted
	 * attempt to do something with all
	 * of the 11/70 parity registers.
	 * In fact, there is little that
	 * can be done.
	 */
	case PRTYFLT:
	case PRTYFLT + USER:
		printf("parity\n");
		if(cputype == 70) {
			logparity(MEMORY);
			for(i=0; i<4; i++)
				printf("%o ", MEMORY->r[i]);
			printf("\n");
			MEMORY->r[2] = -1;
			if(type & USER) {
				i = SIGBUS;
				break;
			}
		}
		panic("parity");
	}
	psignal(u.u_procp, i);

out:
	if(issig())
		psig();
	if(u.u_prof.pr_scale)
		addupc((caddr_t)pc, &u.u_prof, (int)(u.u_stime-syst));
	if (u.u_fpsaved)
		restfp(&u.u_fps);
}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{
}

/*
 * Catch stray interrupts by using trace feature
 * trap through 0 looks like 42
 */
stray(dev, sp, r1, addr)
{
	addr = (addr == 042) ? 0 : addr-2;
	logstray(addr);
	printf("stray interrupt at %o\n", addr);
}
