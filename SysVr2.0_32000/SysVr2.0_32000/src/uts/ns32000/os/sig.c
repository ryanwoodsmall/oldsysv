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
/* @(#)sig.c	6.2 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/text.h>
#include <sys/var.h>
#include <sys/ipc.h>



/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(pgrp, sig)
register pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for(p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
register struct proc *p;
register sig;
{

	sig--;
	if (sig < 0 || sig >= NSIG)
		return;
	p->p_sig |= 1L<<sig;
	if (p->p_stat == SSLEEP && p->p_pri > PZERO)
		setrun(p);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register n;
	register struct proc *p, *q;

	p = u.u_procp;
	while(p->p_sig) {
		n = fsig(p->p_sig);
		if (n == SIGCLD) {
			if (u.u_signal[SIGCLD-1] == 01) {
				for (q = &proc[1];
				 q < (struct proc *)v.ve_proc; q++)
					if (p->p_pid == q->p_ppid &&
					 q->p_stat == SZOMB)
						freeproc(q, 0);
			} else if (u.u_signal[SIGCLD-1])
				return(n);
		} else if (n == SIGPWR) {
			if (u.u_signal[SIGPWR-1] && u.u_signal[SIGPWR-1] != 1)
				return(n);
		} else if (u.u_signal[n-1] != 1 || (p->p_flag&STRC))
			return(n);
		p->p_sig &= ~(1L<<(n-1));
	}
	return(0);
}


/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 */
psig()
{
	register n, p;
	register struct proc *rp;

	rp = u.u_procp;
	if (rp->p_flag&STRC)
		stop();
	n = fsig(rp->p_sig);
	if (n==0)
		return;
	rp->p_sig &= ~(1L<<(n-1));
	if ((p=u.u_signal[n-1]) != 0) {
		if (p == 1)
			return;
		u.u_error = 0;
		if (n != SIGILL && n != SIGTRAP)
			u.u_signal[n-1] = 0;
		sendsig(p, n);
		return;
	}
chk:
	switch(n) {

	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
		if (core(n))
			n += 0200;
		exit(n);
		return;
	}
	exit(n);
}

/*
 * find the signal in bit-position
 * representation in p_sig.
 */
fsig(n)
register int n;
{
	register i;

	for(i = 1; i <= NSIG; i++) {
		if (n & 1L)
			return(i);
		n >>= 1;
	}
	return(0);
}

/*
 * Create a core image on the file "core"
 *
 * It writes USIZE block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
core(n)
{
	register struct inode *ip;
	extern schar();

	if (u.u_uid != u.u_ruid)
		return(0);
	u.u_error = 0;
	if (u.u_fpsaved == 0) {
		savfp(u.u_fps);
		u.u_fpsaved = 1;
	}
	u.u_dirp = "core";
	ip = namei(schar, 1);
	if (ip == NULL) {
		if (u.u_error)
			return(0);
		ip = maknode(0666);
		if (ip==NULL)
			return(0);
	}
	if (!access(ip, IWRITE) && (ip->i_mode&IFMT) == IFREG) {
		coredump(ip, n);
	} else
		u.u_error = EACCES;
	iput(ip);

	return(u.u_error == 0);
}

