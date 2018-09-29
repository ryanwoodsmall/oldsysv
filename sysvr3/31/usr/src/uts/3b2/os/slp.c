/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/slp.c	10.17"
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
	register proc_t *p = u.u_procp;
	register proc_t **q = sqhash(chan);
	register s;
	register tmpdisp;
	int why;

	s = splhi();
	if (panicstr) {
		(void) spl0();
		splx(s);
		return(0);
	}

	/* put on sleep queue */

	ASSERT(secnum(&chan) == 3);
	ASSERT(chan != 0);

	p->p_stat = SSLEEP;
	p->p_wchan = chan;
	p->p_link = *q;
	*q = p;
	if (p->p_time > TZERO)
		p->p_time = TZERO;
	tmpdisp = disp & PMASK;
	if (p->p_pri > tmpdisp)
		p->p_pri = tmpdisp;
	if (tmpdisp > PZERO) {
		p->p_flag &= ~SNWAKE;
		/*
		 * If it's not safe to stop here the caller will have set
		 * PNOSTOP in "disp"; in this case make sure that we won't
		 * stop when issig() is called.
		 */
		if (disp & PNOSTOP) {
			p->p_flag |= SNOSTOP;
			why = JUSTLOOKING;
		} else
			why = FORREAL;
		p->p_flag |= SASLEEP;
 		if (ISSIG(p, why)) {
			/* signal pending: take off sleep queue */
			unsleep(p);
			p->p_stat = SONPROC;
			goto psig;
		}
		/*
		 * We may have stopped in issig(); make sure we're still
		 * on the sleep queue.
		 */
		if (p->p_wchan == 0)
			goto out;
		p->p_stat = SSLEEP;
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		p->p_flag &= ~SASLEEP;
		swtch();
		p->p_flag |= SASLEEP;
 		if (ISSIG(p, why))
			goto psig;
	} else {
		p->p_flag |= SNWAKE;
		swtch();
	}
out:
	p->p_flag &= ~(SNWAKE|SNOSTOP|SASLEEP);
	splx(s);
	return(0);

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * then if PCATCH is set return 1, otherwise do a non-local
	 * jump to the qsav location.
	 */

psig:
	p->p_flag &= ~(SNOSTOP|SASLEEP);
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
 * Wake up all processes sleeping on chan.  Note that a process on a
 * sleep queue may be either in state SSLEEP or in state SSTOP; if it
 * was stopped we remove it from its sleep queue but we don't set it
 * running here.
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
	 * Make arrangements for swapin or preemption if necessary.
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
