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
/* @(#)clock.c	6.3 */
#include "sys/types.h"
#include "sys/psl.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/tuneable.h"
#include "sys/sysmacros.h"
#include "sys/map.h"
#include "sys/swap.h"

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	alarm clock signals
 *	jab the scheduler
 */

#define	PRF_ON	01
extern	prfstat;

time_t	time, lbolt;
int	switching;

int	vhandcnt;	/* counter for t_vhandr */

extern char runin;

clock(isrv, r3, r2, r1, r0, pc, ps)
caddr_t pc;
{
	register struct proc *pp;
	register preg_t	*prp;
	register reg_t	*rp;
	register a;
	register swpt_t *st;
	static short lticks;
	static rqlen, sqlen;
	extern char waitloc;
	extern int freemem; extern vhand();

	/*
	 * if panic stop clock
	 */
	if (panicstr) {
		icudisable(2);
		return;
	}

	/*
	 * profiling
	 */
#ifdef PCS
	pcsintr(pc, ps);
#endif

	/*
	 * if any callout active, update first non-zero time
	 */

	if (callout[0].c_func != NULL) {
		register struct callo *cp;
		cp = &callout[0];
		while(cp->c_time<=0 && cp->c_func!=NULL)
			cp++;
		cp->c_time--;
	}
	if (callout[0].c_time <= 0)
		timepoke();
	if (prfstat & PRF_ON)
		prfintr(pc, ps);
	if (USERMODE(ps)) {
		a = CPU_USER;
		u.u_utime++;
		if (u.u_prof.pr_scale)
			profon();
	} else {
		if (pc == &waitloc) {
			if (syswait.iowait+syswait.swap+syswait.physio) {
				a = CPU_WAIT;
				if (syswait.iowait)
					sysinfo.wait[W_IO]++;
				if (syswait.swap)
					sysinfo.wait[W_SWAP]++;
				if (syswait.physio)
					sysinfo.wait[W_PIO]++;
			} else
				a = CPU_IDLE;
		} else
			a = CPU_KERNAL;
		if (!switching)
			u.u_stime++;
	}
	sysinfo.cpu[a]++;
	pp = u.u_procp;
	if (pp->p_stat==SRUN) {

		/*	Update memory usage for the currently
		 *	running process.
		 */

		for(prp = pp->p_region ; rp = prp->p_reg ; prp++){
			if(rp->r_type == RT_PRIVATE){
				u.u_mem += rp->r_nvalid;
			} else {
				if (rp->r_refcnt == 0) continue;
				u.u_mem += rp->r_nvalid/rp->r_refcnt;
			}
		}
	}
	if (!switching && pp->p_cpu < 80)
		pp->p_cpu++;
	lbolt++;	/* time in ticks */

	if (--lticks <= 0) {
		if (isrv)
			return;
		lticks += HZ;
		++time;
		runrun++;
		minfo.freemem = freemem;

		minfo.freeswap = 0;
		for(st = swaptab; st < &swaptab[MSFILES]; st++) {
			if(st->st_ucnt == NULL)
				continue;
			minfo.freeswap += st->st_nfpgs;
		}
		rqlen = 0;
		sqlen = 0;
		for(pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_stat) {
			if (pp->p_time != SCHMAX)
				pp->p_time++;
			if (pp->p_clktim) 
				if (--pp->p_clktim == 0) 
					psignal(pp, SIGALRM);
			pp->p_cpu >>= 1;
			if (pp->p_pri >= (PUSER-NZERO)) {
				pp->p_pri = (pp->p_cpu>>1) + PUSER +
					pp->p_nice - NZERO;
			}
			if (pp->p_stat == SRUN)
				if (pp->p_flag & SLOAD)
					rqlen++;
				else
					sqlen++;
		}
		if (rqlen) {
			sysinfo.runque += rqlen;
			sysinfo.runocc++;
		}
		if (sqlen) {
			sysinfo.swpque += sqlen;
			sysinfo.swpocc++;
		}

		/*
		 * Wake up paging process every t_vhandr
		 * if memory is running low
		 */
		if (--vhandcnt <= 0) {
			vhandcnt = tune.t_vhandr;
			if(freemem < tune.t_vhandl)
				wakeup((caddr_t)vhand);
		}

		/*
		 * Wakeup sched if
		 * memory is tight or someone is not loaded (runin set)
		 */
		if (runin) {
			runin = 0;
			wakeup(&runin);
		}
	}
}
