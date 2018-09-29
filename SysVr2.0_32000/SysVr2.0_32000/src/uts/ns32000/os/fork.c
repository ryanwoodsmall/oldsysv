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
/* @(#)fork.c	1.5 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/map.h>
#include <sys/seg.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/buf.h>
#include <sys/var.h>
#include <sys/errno.h>
#include <sys/text.h>
#include <sys/proc.h>
#include <sys/debug.h>

extern struct user *u2;
extern struct proc	*lastproc;

int	shmfork();
struct user *makechild();

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc(i)
{
	register struct proc *cp, *pp, *pend;
	register n, a;
	register int (**fptr)();
	extern int (*forkfunc[])();
	static mpid;

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	pp = &proc[0];
	cp = NULL;
	n = (struct proc *)v.ve_proc - pp;
	a = 0;
	do {
		if (pp->p_stat == NULL) {
			if (cp == NULL)
				cp = pp;
			continue;
		}
		if (pp->p_pid==mpid)
			goto retry;
		if (pp->p_uid == u.u_ruid)
			a++;
		pend = pp;
	} while(pp++, --n);
	if (cp==NULL) {
		if ((struct proc *)v.ve_proc >= &proc[v.v_proc]) {
			if (i) {
				syserr.procovf++;
				u.u_error = EAGAIN;
				return(-1);
			} else
				panic("no procs");
		}
		cp = (struct proc *)v.ve_proc;
	}
	if (cp > pend)
		pend = cp;
	pend++;
	v.ve_proc = (char *)pend;
	if (u.u_uid && u.u_ruid) {
		if (cp == &proc[v.v_proc-1] || a > v.v_maxup) {
			u.u_error = EAGAIN;
			return(-1);
		}
	}
	/*
	 * make proc entry for new proc
	 * set flags to include SLOCK so that the scheduler
	 * does not clear the SLOAD. We will be attaching regions
	 * soon, and attachreg increments the region's incore count.
	 */

	pp = u.u_procp;
	cp->p_uid = pp->p_uid;
	cp->p_suid = pp->p_suid;
	cp->p_pgrp = pp->p_pgrp;
	cp->p_nice = pp->p_nice;
	cp->p_size = 0;
	cp->p_stat = SIDL;
	cp->p_clktim = 0;
	cp->p_flag = SLOAD | SLOCK;
	cp->p_pid = mpid;
	cp->p_ppid = pp->p_pid;
	cp->p_time = 0;
	cp->p_cpu = 0;
	cp->p_pri = PUSER + pp->p_nice - NZERO;
	cp->p_frate = 0;


	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if (u.u_ofile[n] != NULL)
			u.u_ofile[n]->f_count++;
	u.u_cdir->i_count++;
	if (u.u_rdir)
		u.u_rdir->i_count++;

	shmfork(cp, pp);

	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(u.u_rsav)) {
		struct proc *p;

		p = lastproc;
		if (p->p_stat == SZOMB) {
			ufree(p->p_addr);
		}
		return(1);
	}
	/*
	 * There is not enough memory for the
	 * page tables for the
	 * new process, this cannot happen here!!
	 */
	if (procdup(cp, pp) < 0) {
		return(-1);
	}

	pp->p_stat = SRUN;
	cp->p_flag &= ~SLOCK;
	cp->p_stat = SRUN;
	setrq(cp);
	u.u_rval1 = cp->p_pid;		/* parent returns pid of child */
	/* have parent give up processor after
	 * its priority is recalculated so that
	 * the child runs first (its already on
	 * the run queue at sufficiently good
	 * priority to accomplish this).  This
	 * allows the dominant path of the child
	 * immediately execing to break the multiple
	 * use of copy on write pages with no disk home.
	 * The parent will get to steal them back
	 * rather than uselessly copying them.
	 */
	runrun = 1;
	return(0);
}

/*
 * create a duplicate copy of a process
 */
procdup(cp, pp)
struct proc *cp, *pp;
{
	register preg_t	*p_prp;
	register preg_t	*c_prp;
	struct user *useg;


	
	/*
	 *	Allocate a u area for the child, and set up memory management
	 */
	
	if ((useg = makechild(pp, cp)) == NULL)
		return(-1);

	/*	Duplicate all the regions of the process.
	 */


	p_prp = pp->p_region;
	c_prp = cp->p_region;

	for( ; p_prp->p_reg ; p_prp++, c_prp++){
		register int	prot;
		register reg_t	*rp;

		prot = (p_prp->p_flags & PF_RDONLY ? SEG_RO : SEG_RW);
		reglock(p_prp->p_reg);
		rp = dupreg(p_prp->p_reg, SEG_CW);
		if(rp == NULL) {
			regrele(p_prp->p_reg);
			if(c_prp > cp->p_region)
				while((--c_prp)->p_reg){
					reglock(c_prp->p_reg);
					u2load(cp->p_addr);
					detachreg(useg, c_prp);
				}
			ufree(cp->p_addr);
			u.u_error = ENOMEM;
			return(-1);
		}

		/* process may have slept - reload mem. mgt. for u */
		u2load(cp->p_addr);
		if(attachreg(rp, useg, p_prp->p_regva,
			     p_prp->p_type, prot) == NULL){
			freereg(rp);
			regrele(p_prp->p_reg);
			if(c_prp > cp->p_region)
				while((--c_prp)->p_reg){
					reglock(c_prp->p_reg);
					u2load(cp->p_addr);
					detachreg(useg, c_prp);
				}
			ufree(cp->p_addr);
			return(-1);
		}

		regrele(p_prp->p_reg);
		regrele(rp);
	}

	return(0);
}

/*
 *	build a u area and address space for the child
 */
struct user *
makechild(pp, cp)
register struct proc *pp, *cp;
{
	label_t	tsav;

	/*
	 *	capture save area for child before entering memory
	 *	allocation routines, which may sleep, thus bashing
	 *	the current save area contents
	 */
	bcopy(u.u_rsav, tsav, sizeof(label_t));

	/*
	 *	Allocate level 1 page table and u-block
	 */
	cp->p_addr = ualloc();
	u2load(cp->p_addr);

	/*
	 *	copy the u area
	 */
	
	bcopy(&u, u2, ptob(USIZE));

	/*
	 *	adjust process context
	 */

	bcopy(tsav, u2->u_rsav, sizeof(label_t));
	u2->u_procp = cp;

	return(u2);
}
