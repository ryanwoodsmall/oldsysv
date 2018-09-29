/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/exit.c	10.11"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/inline.h"


/*
 * exit system call:
 * pass back caller's arg
 */

rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */

exit(rv)
{
	register int i;
	register struct proc *p, *q;
	register preg_t	*prp;
	register reg_t	*rp;
	struct inode	*ip;
	int		flag;

	p = u.u_procp;
	p->p_flag &= ~(STRC);
	p->p_clktim = 0;
	for (i = 0; i < NSIG; i++)
		u.u_signal[i] = SIG_IGN;
	if ((p->p_pid == p->p_pgrp)
	 && (u.u_ttyp != NULL)
	 && (*u.u_ttyp == p->p_pgrp)) {
		*u.u_ttyp = 0;
		signal(p->p_pgrp, SIGHUP);
	}
	for (i = 0; i < v.v_nofiles; i++) {
		if (u.u_ofile[i] != NULL)
			closef(u.u_ofile[i]);
	}
	punlock();
	plock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}

	/*
	 * insert calls to "exitfunc" functions.
	 */
	semexit();
	shmexit();
	hdeexit();

	acct(rv);

	/* free data, text; free U block and stack switch */

	prp = p->p_region;
	while (rp = prp->p_reg) {
		if (ip = rp->r_iptr)
			plock(ip);
		reglock(rp);
		detachreg(prp, &u);
	}

	((struct proc *)p)->p_xstat = rv;
	((struct proc *)p)->p_utime = u.u_cutime + u.u_utime;
	((struct proc *)p)->p_stime = u.u_cstime + u.u_stime;
	flag = 0;

	if ((q = p->p_child) != NULL)
	{
		for (;   ;   q = q->p_sibling) {
			/* loop termination condition tested in last if */
			/* q is child of exiting proc (p) */

			q->p_ppid = 1;
			q->p_parent = &proc[1];
			if (q->p_stat == SZOMB)
				flag = 1;
			else {
				if (q->p_stat == SSTOP && (q->p_flag & STRC))
					setrun(q);
			}

			if (q->p_sibling == NULL) {
				/* attach to proc 1 chain */
				/* lock in case proc 1 is forking now */

				spl6();
				q->p_sibling = proc[1].p_child;
				proc[1].p_child = p->p_child;
				spl0();
				p->p_child = NULL;
				break;
			}
		}	/* end for loop */

		/* only send 1 death of child sig to proc 1--also delay
		 * sending sig until reasonably sure not to sleep on
		 * proc 1 parent-child-sib lock.
		 */

		if (flag) {
			psignal(&proc[1], SIGCLD);
		}
	}	/* end if children */

	/* now take care of all descendants in process group */

	if (p->p_pid == p->p_pgrp) {
		/* only need to worry if exiting proc is a proc
		 * group leader
		 * We do not use parent-child-sibling chain here
		 * since such use would require elaborate locking
		 * of those chains all down the line, and this
		 * case happens infrequently to warrant that
		 * overhead everywhere.
		 */

		for (q = &proc[1];   q < (struct proc *) v.ve_proc; q++) {
			if (p->p_pid == q->p_pgrp)
				q->p_pgrp = 0;
		}
	}

	p->p_pgrp = 0;
	p->p_stat = SZOMB;
	psignal(p->p_parent, SIGCLD);

	ASSERT(noilocks() == 0);

	/* pswtch() frees up stack and ublock */

	swtch();

	/* no deposit, no return */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register struct proc *p;

loop:
	for (p = u.u_procp->p_child ; p ; p = p->p_sibling) {
		if (p->p_stat == SZOMB) {
			freeproc(p, 1);
			return;
		}
		if (p->p_stat == SSTOP) {
			if ((p->p_flag & STRC) && (p->p_flag&SWTED) == 0) {
				p->p_flag |= SWTED;
				u.u_rval1 = p->p_pid;
				u.u_rval2 = (fsig(p)<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if (u.u_procp->p_child) {
		sleep((caddr_t)u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * Remove zombie children from the process table.
 */
freeproc(p, flag)
register struct proc *p;
{

	register proc_t	*q;

	if (flag) {
		register n;

		n = u.u_procp->p_cpu + p->p_cpu;
		if (n > 80)
			n = 80;
		u.u_procp->p_cpu = n;
		u.u_rval1 = p->p_pid;
		u.u_rval2 = ((struct proc *)p)->p_xstat;
	}
	u.u_cutime += ((struct proc *)p)->p_utime;
	u.u_cstime += ((struct proc *)p)->p_stime;
	p->p_stat = NULL;
	p->p_pid = 0;
	p->p_ppid = 0;
	p->p_sig = 0L;
	p->p_flag = 0;
	p->p_wchan = 0;
	p->p_trace = 0;

	q = p->p_parent;
	if (q->p_child == p)
		q->p_child = p->p_sibling;
	else {
		for (q = q->p_child ; q ; q = q->p_sibling)
			if (q->p_sibling == p)
				break;
	
		ASSERT(q  &&  q->p_sibling == p);
		q->p_sibling = p->p_sibling;
	}
}

/* clean up common process stuff -- called from newproc()
 * on error in fork() due to no swap space
 */
pexit()
{	register int	i;

	/* don't include punlock() since not needed for newproc() clean */

	for (i=0; i<v.v_nofiles; i++)
		if (u.u_ofile[i] != NULL) 
			closef(u.u_ofile[i]);
	plock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}

	semexit();
	shmexit();
	hdeexit();
}
