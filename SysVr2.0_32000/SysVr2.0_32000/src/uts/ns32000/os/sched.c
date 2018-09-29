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
/* @(#)sched.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/page.h"
#include "sys/tuneable.h"
#include "sys/region.h"
#include "sys/debug.h"

extern char runin, runout;
extern int freemem;


/*
 * Memory scheduler
 */
sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	int maxbad;
	int tmp;

loop:
	/*
	 * Otherwise, find user to reactivate
	 * Of users ready, select one out longest
	 */
	outage = -20000;
	spl6();
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++)
		if ((rp->p_stat==SXBRK ||
		    (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0)) &&
		    rp->p_time > outage) {
			p = rp;
			outage = rp->p_time;
		}

	/*
	 * If there is no one there, wait.
	 */
	if (outage == -20000) {
		runout++;
		sleep((caddr_t)&runout, PSWP);
		goto loop;
	}
	spl0();

	/*
	 * See if there is memory for that process;
	 * if so, let it go and then delay in order to
	 * let things settle
	 */

	if(freemem >= tune.t_gpgshi) {
		struct pregion	*prp;

		if(p->p_stat == SXBRK) {
			p->p_stat = SRUN;
			setrq(p);
		}
		if ((p->p_flag & SLOAD) == 0)
			if ((prp = findpreg(p, PT_TEXT)) != NULL) {
				reglock(prp->p_reg);
				prp->p_reg->r_incore++;
				regrele(prp->p_reg);
			}
		p->p_flag |= SLOAD;
		p->p_time = 0;
		goto delay;
	}

	/*
	 * none found.
	 * look around for memory.
	 * Select the largest of those sleeping
	 * at bad priority; if none, select the oldest.
	 */


	p = NULL;
	maxbad = 0;
	inage = 0;
	spl6();
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if (rp->p_stat==SZOMB)
			continue;
		if ((rp->p_flag&(SSYS|SLOCK|SLOAD))!=SLOAD)
			continue;
		if (lockedreg(rp))
			continue;
		if (rp->p_stat==SSLEEP || rp->p_stat==SSTOP) {
			tmp = rp->p_pri - PZERO + rp->p_time;
			if (maxbad < tmp) {
				p = rp;
				maxbad = tmp;
			}
		} else
		if (maxbad<=0 && (rp->p_stat==SRUN || rp->p_stat==SXBRK)) {
			tmp = rp->p_time + rp->p_nice - NZERO;
			if (tmp > inage) {
				p = rp;
				inage = tmp;
			}
		}
	}
	spl0();

	/*
	 * Swap out and deactivate process if
	 * sleeping at bad priority, or if it has spent at least
	 * 2 seconds in memory and the other process has spent
	 * at least 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */
	if (maxbad > 0 || (outage>=2 && inage>=2)) {
		struct pregion *prp;

		p->p_flag |= SLOCK;
		if ((prp = findpreg(p, PT_TEXT)) != NULL) {
			ASSERT(prp->p_reg->r_incore > 0);
			reglock(prp->p_reg);
			prp->p_reg->r_incore--;
			regrele(prp->p_reg);
		}
		p->p_flag &= ~SLOAD;
		swapout(p);
		p->p_time = 0;
		p->p_flag &= ~SLOCK;
		goto loop;
	}

	/*
	 * Delay for 1 second and look again later
	 */
delay:
	spl6();
	runin++;
	sleep((caddr_t)&runin, PSWP);
	goto loop;
}

/*	Check if process p has a region locked.  Return
 *	1 is it does, 0 otherwise.  Don't deactivate somebody
 *	with a locked region since it can lead to a deadlock
 */
lockedreg(p)
register struct proc *p;
{
	register preg_t *prp;
	register reg_t *rp;

	/*	Walk through process regions, check for lock flags
	 */
	for(prp = p->p_region; rp = prp->p_reg; prp++)
		if(rp->r_flags & RG_LOCK)
			return(1);
	return(0);
}

/*	Swap out process p
 */
swapout(p)
register struct proc *p;
{
	register preg_t *prp;
	register reg_t *rp;
	int flg;

	ASSERT(p->p_flag & SLOCK);

	/*	Walk through process regions
	 *	Private regions or shared regions that are being used
	 *	exclusively by this process get paged out en masse.
	 *	Other shared regions are just pruned.
	 */
	for(prp = p->p_region; rp = prp->p_reg; prp++) {
		if(rp->r_flags & RG_LOCK)
			continue;
		reglock(rp);

		flg = (rp->r_type == RT_PRIVATE || rp->r_incore == 0);

		/*	if (flg == 0) only take unreferenced pages */
		/*	if (flg == 1) take all valid pages */
		getpages(rp, flg);
		regrele(rp);
	}
}
