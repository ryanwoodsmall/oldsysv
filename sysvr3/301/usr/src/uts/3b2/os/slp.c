/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/slp.c	10.10"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/psw.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/pcb.h"
#include "sys/signal.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/inline.h"

/*
 * sleep-wakeup hashing:  Each entry in hsque[] points
 * to a linked list of sleeping processes.   NHSQUE must
 * be a power of 2.  Sqhash(x) is used to index into
 * hsque[] based on the sleep channel.
 */

#define NHSQUE		64
#define sqhash(X)	(&hsque[((int)X >> 3) & (NHSQUE-1)])

proc_t			*hsque[NHSQUE];

extern int		userstack[];

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * When pri > PZERO, SNWAKE bit in p_flag will be set
 * to indicate no disturbance during sleep.
 * This is different from the old way that compares
 * p_pri and PZERO.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */

#define	TZERO	10

sleep(chan, disp)
caddr_t chan;
{
	register proc_t *rp = u.u_procp;
	register proc_t **q = sqhash(chan);
	register s;
	register tmpdisp;

	s = splhi();
	if (panicstr) {
		spl0();
		splx(s);
		return(0);
	}

	/* put on sleep queue */

	ASSERT(secnum(&chan) == 3);

	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_link = *q;
	*q = rp;
	if (rp->p_time > TZERO)
		rp->p_time = TZERO;
	tmpdisp = disp & PMASK;
	if(rp->p_pri > tmpdisp)
		rp->p_pri = tmpdisp;
	if (tmpdisp > PZERO) {
		rp->p_flag &= ~SNWAKE;
 		if ((rp->p_flag & SPRSTOP) || (rp->p_sig && issig())) {

			/* signal pending: take off sleep queue */

			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			*q = rp->p_link;
			goto psig;
		}
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		swtch();
 		if ((rp->p_flag & SPRSTOP) || (rp->p_sig && issig()))
			goto psig;
	} else {
		rp->p_flag |= SNWAKE;
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
	if (disp & PCATCH)
		return(1);

	MONITOR('L', u.u_qsav[7], 0, 0, 0);

	longjmp(u.u_qsav);

	/* NOTREACHED */
}

/*
 * Remove a process from its wait queue.
 */
unsleep(p)
register struct proc *p;
{
	register struct proc **q;
	register s;

	s = splhi();
	if (p->p_wchan) {
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link)
			;
		*q = p->p_link;
		p->p_wchan = 0;
	}
	splx(s);
}

/*
 * Wake up all processes sleeping on chan.
 */

wakeup(chan)
register caddr_t chan;
{
	register proc_t *p;
	register proc_t	**q;
	register int	runsched = 0;
	register int	s;

	s = splhi();
	for (q = sqhash(chan); p = *q; ) {
		ASSERT(p->p_stat == SSLEEP || p->p_stat == SSTOP);
#if 1
		if (p->p_stat != SSLEEP && p->p_stat != SSTOP)
			panic("wakeup p_stat");
#endif
		if (p->p_wchan == chan) {
			/* 
			 * take off sleep queue, put on run queue
			 * if not SSTOP.
			 */
			p->p_wchan = 0;
			*q = p->p_link;
			if (p->p_stat == SSLEEP) {
				p->p_stat = SRUN;
				setrq(p);

				MONITOR('W', p, chan, 0, 0);

				/*
				 * Make arrangements for swapin
				 * or preemption if necessary
				 */

				if (!(p->p_flag&SLOAD)) {

					/* we must not call setrun here!*/

					p->p_time = 0;
					if (runout > 0)
						runsched = 1;
				} else if (p->p_pri < curpri)
					runrun++;
			}
		} else
			q = &p->p_link;
	}

	if (runsched) {
		runout = 0;
		setrun(&proc[0]);
	}
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */

setrun(p)
register struct proc *p;
{
	register s;

	s = splhi();
	if (p->p_stat == SSLEEP || p->p_stat == SSTOP) {

		/* take off sleep queue */

		unsleep(p);
	} else if (p->p_stat == SRUN) {

		/* already on run queue */

		splx(s);
		return;
	}

	/* put on run queue */

	ASSERT(p->p_wchan == 0);
	p->p_stat = SRUN;
	setrq(p);

	/*
	 * Make arrangements for swapin
	 * or preemption if necessary
	 */

	if (!(p->p_flag&SLOAD)) {
		p->p_time = 0;
		if (runout > 0) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else if (p->p_pri < curpri)
		runrun++;

	splx(s);
}
