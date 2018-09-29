/* @(#)syscall.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/sysinfo.h"
#include "sys/psl.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/mtpr.h"

#define NSYSENT 64

/*
 * Called from the trap handler when a processor trap occurs.
 */
int	scallno;
syscall(sp, type,  code, pc, ps)
register code;
{
	register struct user *up;
	register struct proc *pp;
	register i;
	register struct sysent *callp;
	register struct systmcall *scp;
	int	*a;
	time_t syst;

	up = &u;
	syst = up->u_stime;
	up->u_ar0 = (int *)(0x80000000 - 18*4);
	sysinfo.syscall++;
	up->u_error = 0;
	ps &= ~PS_C;
	a = (int *)u.u_ar0[AP];
	a++;			/* skip word with param count */
	i = code&0377;
	if (i >= NSYSENT)
		i = 0;
	else if (i==0) {	/* indirect */
		i = fuword(a++)&0377;
		if (i >= NSYSENT)
			i = 0;
	}
	callp = &sysent[i];
	for(i=0; i<callp->sy_narg; i++) {
		u.u_arg[i] = fuword(a++);
	}
	up->u_dirp = (caddr_t)up->u_arg[0];
	up->u_rval1 = 0;
	up->u_rval2 = up->u_ar0[R1];
	up->u_ap = up->u_arg;
	if (setjmp(up->u_qsav)) {
		spl0();
		if (up->u_error==0)
			up->u_error = EINTR;
	} else {
		(*callp->sy_call)();
	}
	if (up->u_error) {
		up->u_ar0[R0] = up->u_error;
		ps |= PS_C;	/* carry bit */
		if (++up->u_errcnt > 16) {
			up->u_errcnt = 0;
			runrun++;
		}
	} else {
		up->u_ar0[R0] = up->u_rval1;
		up->u_ar0[R1] = up->u_rval2;
	}
	pp = up->u_procp;
	resetpri(pp, i);

	if (runrun != 0)
		qswtch();

	if (pp->p_sig && issig())
		psig(code);
	if (up->u_prof.pr_scale)
		addupc((caddr_t)up->u_ar0[PC], &up->u_prof, (int)(up->u_stime-syst));
}

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
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}
