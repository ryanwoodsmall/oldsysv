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
/* @(#)slp.c	6.4 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/buf.h>
#include <sys/var.h>
#include <sys/ipc.h>
#include <sys/errno.h>


/*
 *	sleep-wakeup hashing: each entry in hsque[] points to a linked list
 *	of sleeping processes. NHSQUE must be a power of 2. Sqhash(x)
 *	is used to index into hsque[] based on the sleep channel
 */
#define NHSQUE	64		/* must be power of 2 */
#define sqhash(X)		(&hsque[((int)X >> 3) & (NHSQUE-1)])

struct proc *hsque[NHSQUE];

char	runout, runin, runrun, curpri;
struct proc *curproc, *runq;

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
#define	TZERO	10

sleep(chan, disp)
caddr_t chan;
{
	register struct proc *rp = u.u_procp;
	register struct proc **q = sqhash(chan);
	register s;

	s = splhi();
	if (panicstr) {
		spl0();
		splx(s);
		return(0);
	}
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_link = *q;
	*q = rp;
	if (rp->p_time > TZERO)
		rp->p_time = TZERO;
	if ((rp->p_pri = (disp&PMASK)) > PZERO) {
		if (rp->p_sig && issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			*q = rp->p_link;
			spl0();
			goto psig;
		}
		spl0();
		swtch();
		if (rp->p_sig && issig())
			goto psig;
	} else {
		spl0();
		swtch();
	}
	splx(s);
	return(0);

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * if PCATCH is set, return 1, else
	 * execute non-local goto to the qsav location.
	 */
psig:
	splx(s);
	if (disp&PCATCH)
		return(1);
	longjmp(u.u_qsav);
	/* NOTREACHED */
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
register caddr_t chan;
{
	register struct proc *p;
	register struct proc **q;
	register runsched = 0;
	register s;


	s = splhi();
	for (q = sqhash(chan); p = *q; ) {
		if (p->p_wchan==chan && p->p_stat==SSLEEP) {
			p->p_stat = SRUN;
			p->p_wchan = 0;
			/*
			 * take off sleep queue, put on run queue
			 */
			*q = p->p_link;
			p->p_link = runq;
			runq = p;
			if (!(p->p_flag&SLOAD)) {
				p->p_time = 0;
				/*
				 * defer setrun to avoid breaking link chain
				 */
				if (runout)
					runsched = 1;
			} else if (p->p_pri < curpri)
				runrun++;
		} else {
			q = &p->p_link;
		}
	}
	if (runsched) {
		runout = 0;
		setrun(&proc[0]);
	}
	splx(s);
}

setrq(p)
register struct proc *p;
{
	register struct proc *q;
	register s;

	s = splhi();
	for(q=runq; q!=NULL; q=q->p_link)
		if (q == p) {
			printf("proc on q\n");
			goto out;
		}
	p->p_link = runq;
	runq = p;
out:
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
register struct proc *p;
{
	register struct proc **q;
	register s;

	s = splhi();
	if (p->p_stat == SSLEEP) {
		/* take off sleep queue */
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link);
		*q = p->p_link;
		p->p_wchan = 0;
	} else if (p->p_stat == SRUN) {
		/* already on run queue - just return */
		splx(s);
		return;
	}
	/* put on run queue */
	p->p_stat = SRUN;
	p->p_link = runq;
	runq = p;
	if (!(p->p_flag&SLOAD)) {
		p->p_time = 0;
		if (runout) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else if (p->p_pri < curpri)
		runrun++;
	splx(s);
}
