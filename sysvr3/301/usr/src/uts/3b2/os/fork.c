/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/fork.c	10.14"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/acct.h"
#include "sys/tuneable.h"
#include "sys/inline.h"

#define NOFORCE	0

/*	The following is used to lock the kernel window which
**	is used to address a u-block other than the u-block of
**	the current process.  This lock is needed since
**	procdup can sleep while using the window.
*/

int	win_ublk_lock;

winublock()
{
	while (win_ublk_lock)
		sleep(&win_ublk_lock, PZERO);
	win_ublk_lock = 1;
}

winubunlock()
{
	win_ublk_lock = 0;
	wakeup(&win_ublk_lock);
}

winublocked()
{
	return(win_ublk_lock);
}

/*
 * fork system call.
 */

fork()
{
	sysinfo.sysfork++;

	/*
	 * Disallow if
	 *  No processes at all; or
	 *  not su and too many procs owned; or
	 *  not su and would take last slot; or
	 * Check done in newproc().
	 */

	switch (newproc(1)) {
		case 1:  /* child -- successful newproc */
			u.u_rval1 = u.u_procp->p_ppid;
			u.u_rval2 = 1; /* child */
			u.u_start = time;
			u.u_ticks = lbolt;
			u.u_mem = u.u_procp->p_size;
			u.u_ior = u.u_iow = u.u_ioch = 0;
			u.u_cstime = 0;
			u.u_stime = 0;
			u.u_cutime = 0;
			u.u_utime = 0;
			u.u_acflag = AFORK;
			u.u_lock = 0;
			ASSERT(noilocks() == 0);
			return;
		case 0: /* parent, rval1 setup by newproc */
			/* u.u_rval1 = pid_of_child; */
			break;
		default:	/* couldn't fork */
			u.u_error = EAGAIN;
			break;
	}

	u.u_rval2 = 0;	/* parent */
}


/*
 * Create a new process-- the internal version of
 * sys fork.
 *
 * This changes the new proc structure and
 * alters only u.u_procp kf the uarea.
 *
 * It returns 1 in the new process, 0 in the old.
 */

int	mpid;

newproc(failok)
{
	register struct proc *cp, *pp, *pend;
	register n, a;
	extern struct kpcb kpcb_pdup;

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
		if (pp->p_pid == mpid)
			goto retry;
		if (pp->p_uid == u.u_ruid)
			a++;
		pend = pp;
	} while (pp++, --n);
	if (cp == NULL) {
		if ((struct proc *)v.ve_proc >= &proc[v.v_proc]) {
			if (failok) {
				syserr.procovf++;
				u.u_error = EAGAIN;
				return(-1);
			} else
				cmn_err(CE_PANIC, "newproc - no procs");
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
	 */
	pp = u.u_procp;
	cp->p_uid = pp->p_uid;
	cp->p_suid = pp->p_suid;
	cp->p_sgid = pp->p_sgid;
	cp->p_pgrp = pp->p_pgrp;
	cp->p_nice = pp->p_nice;
	cp->p_chold = pp->p_chold;
	cp->p_sig = pp->p_sig;
	cp->p_hold = pp->p_hold;
	cp->p_stat = SIDL;
	cp->p_clktim = 0;
	cp->p_flag = SLOAD | (pp->p_flag & (SSEXEC|SPROCTR));
	cp->p_pid = mpid;
	cp->p_epid = mpid;
	cp->p_ppid = pp->p_pid;
	cp->p_time = 0;
	cp->p_cpu = 0;
	cp->p_pri = PUSER + pp->p_nice - NZERO;
	cp->p_sigmask = pp->p_sigmask;
	cp->p_srama[SCN2 - SCN2] = NULL;
	cp->p_srama[SCN3 - SCN2] = NULL;
	*(int *)&cp->p_sramb[SCN2 - SCN2] = -1;
	*(int *)&cp->p_sramb[SCN3 - SCN2] = -1;

	/* link up to parent-child-sibling chain---
	 * no need to lock generally since only a free proc call
	 * (done by same parent as newproc) diddles with child chain.
	 */

	cp->p_sibling = pp->p_child;
	cp->p_parent = pp;
	pp->p_child = cp;
	cp->p_sysid = pp->p_sysid;	/* REMOTE */

	/*
	 * make duplicate entries
	 * where needed
	 */

	for (n=0; n<v.v_nofiles; n++)
		if (u.u_ofile[n] != NULL) 
			u.u_ofile[n]->f_count++;

	plock(u.u_cdir);
	u.u_cdir->i_count++;
	prele(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);	/*lock root directory*/
		u.u_rdir->i_count++;
		prele(u.u_rdir);
	}
	
	/*
	 * Copy process.
	 */
	switch (procdup(cp, pp)) {
	case 0:
		/* Successful copy */
		break;
	case -1:
		if (!failok)
			cmn_err(CE_PANIC, "newproc - fork failed\n");

		/* reset all incremented counts */

		pexit();

		/* clean up parent-child-sibling pointers--
		* No lock necessary since nobody else could
		* be diddling with them here.
		*/

		pp->p_child = cp->p_sibling;
		cp->p_parent = NULL;
		cp->p_sibling = NULL;
		cp->p_stat = NULL;
		cp->p_ppid = 0;
		u.u_error = EAGAIN;
		return(-1);
	case 1:
		/* Child resumes here */
		return(1);
	}

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

extern int mau_present;

/*
 * Create a duplicate copy of a process, everything but stack.
 */

procdup(cp, pp)
struct proc	*cp;
struct proc	*pp;
{
	register preg_t		*p_prp;
	register preg_t		*c_prp;
	register user_t		*uservad;
	register int		*sp;
	register inode_t	*ip;
	register int		n2seg;
	register int		n3seg;
	user_t			*uballoc();
	int			tmppsw;

	/* 	Allocate SDT tables and ublock for the child
	*/
	if (u.u_dmm)
		dmmdet(&u);

	winublock();
	n2seg = (*(int *)&pp->p_sramb[0] == -1)  ?
					0 : (pp->p_sramb[0].SDTlen) + 1;
	n3seg = (*(int *)&pp->p_sramb[1] == -1)  ?
					0 : (pp->p_sramb[1].SDTlen) + 1;
	if ((uservad = uballoc(cp,n2seg,n3seg)) == NULL) {
		winubunlock();
		if (u.u_dmm)
			dmmatt(&u);
		return(-1);
	}

	/*	Setup child u-block
	 */
	
	setuctxt(cp, uservad);

	/*	Duplicate all the regions of the process.
	 */

	p_prp = pp->p_region;
	c_prp = cp->p_region;

	for (; p_prp->p_reg ; p_prp++, c_prp++) {
		register int		prot;
		register reg_t		*rp;

		prot = (p_prp->p_flags & PF_RDONLY ? SEG_RO : SEG_RW);
		if (ip = p_prp->p_reg->r_iptr)
				plock(ip);
		reglock(p_prp->p_reg);
		if ((rp = dupreg(p_prp->p_reg, NOSLEEP, NOFORCE)) == NULL) {
			regrele(p_prp->p_reg);
			if (c_prp > cp->p_region)
				do {
					c_prp--;
					if (ip = c_prp->p_reg->r_iptr) 
						plock(ip);
					reglock(c_prp->p_reg);
					detachreg(c_prp, uservad);
				} while (c_prp > cp->p_region);
			winubunlock();
			ubfree(cp);
			if (u.u_dmm)
				dmmatt(&u);
			return(-1);
		}
		if (attachreg(rp, uservad, p_prp->p_regva,
			     p_prp->p_type, prot) == NULL) {
			if (rp->r_refcnt)
			{
				if (ip = rp->r_iptr)
					prele(ip);
				regrele(rp);
			}
			else
				freereg(rp);
			if (rp != p_prp->p_reg) {
				regrele(p_prp->p_reg);

				/* Note that we don't want to
				** do a prele(ip) here since
				** rp will have had the same
				** ip value and the freereg
				** will have unlocked it.
				*/
			}
			if (c_prp > cp->p_region)
				do {
					c_prp--;
					if (ip = c_prp->p_reg->r_iptr) 
						plock(ip);
					reglock(c_prp->p_reg);
					detachreg(c_prp, uservad);
				} while (c_prp > cp->p_region);
			winubunlock();
			ubfree(cp);
			if (u.u_dmm)
				dmmatt(&u);
			return(-1);
		}
		if (ip)
			prele(ip);
		regrele(p_prp->p_reg);
		if (rp != p_prp->p_reg) {
			regrele(rp);
		}
	}

	/*	Flush the parents regions so that any
	**	pages which were made copy-on-write
	**	get flushed from cache.
	*/

	flushsect(SCN2);
	flushsect(SCN3);

	/*	Set up values for child to
	**	return to "newproc".
	*/

	ASSERT(u.u_pcbp == (pcb_t *)&u.u_kpcb.psw);

	sp = (int *)((char *)&pp + 4);
	movpsw(tmppsw);
	*(int *)&uservad->u_kpcb.psw = tmppsw;
	uservad->u_kpcb.pc = (int (*)())sp[0];
	uservad->u_kpcb.sp = (int *)&cp;
	uservad->u_kpcb.regsave[K_AP] = sp[1];
	uservad->u_kpcb.regsave[K_FP] = sp[2];
	uservad->u_kpcb.regsave[K_R0] = 1;
	uservad->u_kpcb.regsave[K_R3] = sp[3];
	uservad->u_kpcb.regsave[K_R4] = sp[4];
	uservad->u_kpcb.regsave[K_R5] = sp[5];
	uservad->u_kpcb.regsave[K_R6] = sp[6];
	uservad->u_kpcb.regsave[K_R7] = sp[7];
	uservad->u_kpcb.regsave[K_R8] = sp[8];
	
	/*	Reconnect any doubly mapped segments.
	*/

	if (u.u_dmm) {
		dmmatt(uservad);
		dmmatt(&u);
	}

	/*	Put the child on the run queue.
	*/

	winubunlock();
	cp->p_stat = SRUN;
	setrq(cp);
	return(0);
}

/*	Setup context of child process
*/

setuctxt(p, up)
register struct proc *p;	/* child proc pointer */
register struct user *up;	/* child u-block pointer */
{
	ASSERT(winublocked());

	/* save the current mau status for the child */

	if (mau_present)
		mau_save();

	/*	Copy u-block 
	 */

	bcopy(&u, up, ctob(USIZE));


	/*	Reset u_procp.
	*/

	up->u_procp = p;
}

/*
 * uballoc(p,n2seg,n3seg) 
 *	allocates section 2 and 3 SDT table, PDT table for ublock
 *	and ublock for proc p.
 *	n2seg = number of SDT[2] entries
 *	n3seg = number of SDT[3] entries
 */

struct user *
uballoc(p,n2seg,n3seg)
register proc_t *p;
int n2seg, n3seg;
{
	uint s3table;
	char *addr;
	sde_t *st;
	char *getcpages();

	ASSERT(winublocked());

	/* 	Allocate section 2 and 3 SDT tables for the child
	*/

	if (n2seg > 0)
		if (growsdt(p, 2, n2seg, NOSLEEP))
			return(NULL);
	if (growsdt(p, 3, n3seg, NOSLEEP)) {
		growsdt(p,2,0,NOSLEEP);
		return(NULL);
	}


	s3table = phystokv(p->p_srama[SCN3 - SCN2]);

	
	/* 	Allocate pages for ublock.
	*/

	if (availrmem - USIZE < tune.t_minarmem ||
	   availsmem - USIZE < tune.t_minasmem) {
		nomemmsg("uballoc", USIZE, 0, 0);
		growsdt(p, 2, 0, NOSLEEP);
		growsdt(p, 3, 0, NOSLEEP);
		return(NULL);
	} else {
		availrmem -= USIZE;
		availsmem -= USIZE;
	}

	reglock(&sysreg);
	memlock();
	if (ptmemall(&sysreg, ubptbl(p), USIZE, 1, 0)) {
		cmn_err(CE_PANIC,
			"uballoc - ptmemall failed for u-block");
	}
	memunlock();
	regrele(&sysreg);
	addr = (char *)win_ublk;
	kvtokstbl(addr)->wd2.address = kvtophys(ubptbl(p));
	flushmmu(addr, USIZE);

	/* 	Set the segment tabe entry for u-block
	*/
	st = (sde_t *)s3table;
	st->wd2.address = kvtophys(ubptbl(p));
	st->seg_prot = KRWE;
	st->seg_flags = SDE_flags;
	st->seg_len = ctomo(USIZE);

	return((struct user *)addr);
}

/*
 *	ubfree(P) frees up proc p's SDT tables and ublock
 */

ubfree(p)
register struct proc *p;
{
	/* free up u_block */

	reglock(&sysreg);
	memlock();
	pfree(&sysreg, ubptbl(p), 0, USIZE);
	memunlock();
	regrele(&sysreg);
	availrmem += USIZE;
	availsmem += USIZE;

	/* free SDT tables */

	growsdt(p, 2, 0, 0);	/* SDT[2] */
	growsdt(p, 3, 0, 0);	/* SDT[3] */
}

