/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
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

#define NSYSENT 64

/*
 * Called from the trap handler when a processor trap occurs.
 */
syscall(r3, r2, r1, sp, r0, type, pc, psr_mod)
{
	register struct proc *pp;
	register i;
	register struct sysent *callp;
	register int *a;
	time_t syst;

	syst = u.u_stime;
	u.u_ar0 = (int *)((int)&u + ptob(USIZE) - 4*4);
	sysinfo.syscall++;
	u.u_fpsaved = 0;
	u.u_error = 0;
	psr_mod &= ~PS_F;
	a = (int *)r1;
	pc++;	/* move past svc */
	i = r0&0377;
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
	u.u_dirp = (caddr_t)u.u_arg[0];
	u.u_rval1 = 0;
	u.u_rval2 = r1;
	u.u_ap = u.u_arg;
	if (setjmp(u.u_qsav)) {
		spl0();
		if (u.u_error==0)
			u.u_error = EINTR;
	} else {
		(*callp->sy_call)();
	}
	if (u.u_error) {
		r0 = u.u_error;
		psr_mod |= PS_F;
		if (++u.u_errcnt > 16) {
			u.u_errcnt = 0;
			runrun++;
		}
	} else {
		r0 = u.u_rval1;
		r1 = u.u_rval2;
	}
	pp = u.u_procp;
	calcppri(pp, i);

	if (runrun != 0)
		qswtch();

	if (pp->p_sig && issig())
		psig();
	if (u.u_prof.pr_scale)
		addupc((caddr_t)pc, &u.u_prof, (int)(u.u_stime-syst));
	if (u.u_fpsaved)
		restfp(u.u_fps);
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
