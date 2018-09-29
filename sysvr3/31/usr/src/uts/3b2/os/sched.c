/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sched.c	10.5.1.2"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/tuneable.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/buf.h"

extern unsigned int	sxbrkcnt;

/*	The scheduler sleeps on runout when there is no one
**	to swap in.  It sleeps on runin when it could not 
**	find space to swap someone in or after swapping
**	someone in.
*/

char	runout;
char	runin;

/*	We allocate a single page table, dbd pair at system
**	startup and use this in swapin and swapout to swap
**	the u-block.  The following points to this area.
*/

pde_t	*sched_ptp;
pglst_t	sched_pgl[USIZE];

/*
 * Memory scheduler
 */

sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	register int	ii;
	int maxbad;
	int tmp;
	int runable;
	proc_t	*justloaded;

	/*	Allocate a page table and dbd to use for swapping
	**	the u-block in and out.
	*/

	if((sched_ptp = (pde_t *)uptalloc(2)) == NULL)
		cmn_err(CE_PANIC,
			"sched - couldn't get ptbl and dbd for u-block");
	for(ii = 0  ;  ii < USIZE  ;  ii++)
		sched_pgl[ii].gp_ptptr = &sched_ptp[ii];

loop:
	/*
	 * find user to swap in;
	 * of users ready, select one out longest
	 */

	spl6();
	outage = -20000;
	runable = 0;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if ((rp->p_stat==SXBRK ||
		    (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0)) &&
		    rp->p_time > outage) {
			p = rp;
			outage = rp->p_time;
		}
		if (rp->p_stat == SRUN  &&
		   (rp->p_flag & (SLOAD | SSYS)) == SLOAD)
			runable = 1;
	}

	/*
	 * If there is no one there, wait.
	 */
	if (outage == -20000) {
		runout++;
		sleep(&runout, PSWP);
		goto loop;
	}
	spl0();

	/*
	 * See if there is memory for that process;
	 * if so, let it go and then delay in order to
	 * let things settle
	 */

	justloaded = NULL;

	if (!runable  ||  freemem > tune.t_gpgslo) {
		if (p->p_stat == SXBRK) {
			ASSERT(p->p_wchan == 0);
			p->p_stat = SRUN;
			--sxbrkcnt;
			setrq(p);
		}
		if (!(p->p_flag & SLOAD)){
			if(!swapinub(p, 1)) 
				goto unload;
		}
		
		p->p_flag |= SLOAD;
		p->p_time = 0;

		if (freemem > tune.t_gpgslo)
			goto delay;
		justloaded = p;
	} 

	/*
	 * none found.
	 * look around for memory.
	 * Select the largest of those sleeping
	 * at bad priority; if none, select the oldest.
	 */


   unload:

	p = NULL;
	maxbad = 0;
	inage = 0;
	spl6();
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if (rp->p_stat==SZOMB)
			continue;
		if ((rp->p_flag&(SSYS|SLOAD|SLOCK))!=SLOAD)
			continue;
		if (rp == justloaded)
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
		if (maxbad > 0) {
			if (!(p->p_stat == SSLEEP || p->p_stat == SSTOP))
				goto loop;
		} else if (p->p_stat != SRUN  &&  p->p_stat != SXBRK) {
			goto loop;
		}
		if (p->p_flag & SLOCK)
			goto loop;

		p->p_flag &= ~SLOAD;
		if (!swapout(p)) {
			p->p_flag |= SLOAD;
			goto delay;
		} else {
			p->p_time = 0;
			goto loop;
		}
			
	}

	/*
	 * Delay for 1 second and look again later
	 */
delay:
	spl6();
	runin++;
	sleep(&runin, PSWP);
	goto loop;
}


/*
 * Swap out process p.
 */

int
swapout(p)
register struct proc *p;
{
	register preg_t	*prp;
	register reg_t *rp;
	register int flg;
	register int rtn; 
	register pde_t *pt;
	register dbd_t *dp;


	/*	Walk through process regions
	 *	Private regions or shared regions that are being used
	 *	exclusively by this process get paged out en masse.
	 *	Other shared regions are just pruned.
	 *
	 *	return 1 - every region gets paged out 
	 *      return 0 - find region(s) locked
	 */

	rtn = 1;
	pglstlk();

	for (prp = p->p_region; rp = prp->p_reg; prp++) {
		if (rp->r_flags & RG_LOCK) {
			rtn = 0;
			continue;
		}

		reglock(rp);
		flg = (rp->r_type == RT_PRIVATE || rp->r_refcnt == 1);

		/*	if (flg == 0) only take unreferenced pages */
		/*	if (flg == 1) take all valid pages */

		getpages(rp, flg);
		if (gprglst[gprgndx].gpr_count == 0  ||
		   gprglst[gprgndx].gpr_rgptr != rp) 
			regrele(rp);
	}

	/*	If we still do not have enough free
	**	memory and there are pages on the
	**	lists to steal, then steal them now 
	**	before we go to sleep.  Otherwise, 
	**	unlock all of the locked regions.
	*/

	if (fpglndx) 
		freechunk(0); 

	if (spglndx)
		swapchunk(0, 1);

	ASSERT(spglndx == 0);
	ASSERT(fpglndx == 0);
	ASSERT(gprglst[gprgndx].gpr_count == 0);

	pglstunlk();

	/*
	 * If process is not completely swapped-out, don't swap out
	 * the u-Block.
	 */

	if (!rtn)
		return(rtn);
	
	if (p->p_flag & SPROCIO)
		return(1);

	/*	Copy the u-block.  First get a pointer to the
	**	page table for the u-block and copy it and the
	**	dbd into our temporary area.
	*/

	pt = ubptbl(p);
	dp = (dbd_t *)&sched_ptp[NPGPT];
	bcopy(pt, sched_ptp, USIZE*NBPW);
	dp->dbd_type = DBD_NONE;

	/*	Allocate swap space for the u-block and swap
	**	it out.
	*/

	if(swalloc(sched_pgl, USIZE, 0) < 0)
		return(0);
	ubslock(p);
	swap(sched_pgl, USIZE, B_WRITE);

	/*	Free up the memory being used by the u-block.
	*/

	reglock(&sysreg);
	memlock();
	pfree(&sysreg, sched_ptp, NULL, USIZE);
	memunlock();
	regrele(&sysreg);

	/*	Copy back the page table and dbd.
	*/

	bcopy(sched_ptp, pt, USIZE*NBPW);
	bcopy(dp, p->p_ubdbd, USIZE*NBPW);
	p->p_flag &= ~SULOAD;
	ubsrele(p);
	return(1);
}


/*
 * Swap in a process's u-block.
 */

int
swapinub(pp, nosleep)
register proc_t	*pp;
register nosleep;
{
	register pde_t	*pt;
	register dbd_t	*dp;

	
	/*	Swap in  the u-block.  First get a pointer to the
	**	page table for the u-block and copy it and the
	**	dbd into our temporary area.
	*/

	ubslock(pp);

	/* If U-Block is already in-core, return */
	if (pp->p_flag & SULOAD) {
		ubsrele(pp);
		return(1);
	}
	pt = ubptbl(pp);
	dp = (dbd_t *)&sched_ptp[NPGPT];
	bcopy(pt, sched_ptp, USIZE*NBPW);
	bcopy(pp->p_ubdbd, dp, USIZE*NBPW);
	ASSERT(dp->dbd_type == DBD_SWAP);

	/*	Allocate memory for the u-block.
	*/

	reglock(&sysreg);
	memlock();
	if(ptmemall(&sysreg, sched_ptp, USIZE, 1, nosleep) < 0){
		memunlock();
		regrele(&sysreg);
		ubsrele(pp);
		return(0);
	}
	memunlock();
	regrele(&sysreg);

	/*	Swap in the u-block and free up the swap space.
	**	We could keep the swap space allocated here and
	**	check for it in swapout.  Then just free it
	**	in exit.  The problem is that the process could
	**	then have to use both swap and real memory at
	**	the same time which would break the deadlock
	**	prevention code.
	*/

	swap(sched_pgl, USIZE, B_READ);
	{
		register i;
		for (i=0; i< USIZE; i++)
		{
			swfree1(&dp[i]);
			(&dp[i])->dbd_type = DBD_NONE;
		}
	}

	/*	Copy back the page table and dbd.
	*/

	bcopy(sched_ptp, pt, USIZE*NBPW);
	bcopy(dp, pp->p_ubdbd, USIZE*NBPW);
	pp->p_flag |= SULOAD;
	ubsrele(pp);
	return(1);
}

/*
 * Lock arbitration to ensure that we don't try to swap out
 * the u-block at the same time as it's being swapped in.
 */
ubslock(p)
register proc_t *p;
{
	while (p->p_flag & SUSWAP) {
		p->p_flag |= SUWANT;
		sleep(((caddr_t)&p->p_flag)+1, PSWP);
	}
	p->p_flag |= SUSWAP;
}

ubsrele(p)
register proc_t *p;
{
	p->p_flag &= ~SUSWAP;
	if (p->p_flag & SUWANT) {
		p->p_flag &= ~SUWANT;
		wakeup(((caddr_t)&p->p_flag)+1);
	}
}
