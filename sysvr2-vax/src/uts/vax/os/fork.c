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
#include <sys/mtpr.h>
#include <sys/debug.h>

extern spte_t *userpte;
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
		struct pregion *prp;
		extern struct user	*uservad;

		p = lastproc;
		switch(p->p_stat) {
		case SZOMB:
			if ((prp = findpreg(p, PT_STACK)) == NULL) {
				panic("fork: findpreg");
			}
			reglock(prp->p_reg);
			uaccess(&(baseseg[p->p_ptaddr+p->p_ptsize][-USIZE]) );
			detachreg(uservad, prp);
			sptfree(vtop(p->p_ptaddr, 0), p->p_ptsize, 0);
			break;
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
	register int i;
	register pte_t *ppt,  *cpt;
	extern struct user	*uservad;
	pt_t *seg;
	struct user *useg;
	pte_t *upt, *spt, tmp;


	
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
					detachreg(useg, c_prp);
				}
			sptfree(useg, USIZE, 1);
			u.u_error = ENOMEM;
			return(-1);
		}

		if(attachreg(rp, useg, p_prp->p_regva,
			     p_prp->p_type, prot) == NULL){
			freereg(rp);
			regrele(p_prp->p_reg);
			if(c_prp > cp->p_region)
				while((--c_prp)->p_reg){
					reglock(c_prp->p_reg);
					detachreg(useg, c_prp);
				}
			sptfree(useg, USIZE, 1);
			return(-1);
		}

		regrele(p_prp->p_reg);
		regrele(rp);
	}

	/*
	 * drop the u area into the stack region
	 */
	
	c_prp = findpreg(cp, PT_STACK);
	reglock(c_prp->p_reg);
	upt = &(baseseg[cp->p_ptaddr + cp->p_ptsize][-USIZE]);
	spt = (pte_t *)(&sbrpte[svtop(useg)]);
	cp->p_addr = (short)spt->pgm.pg_pfn;
	for (i=0; i<USIZE; i++) {
		tmp = *upt;
		*upt = *spt;
		upt->pgm.pg_lock = PT_LOCK;
		upt++;
		*spt++ = tmp;
	}
	sptfree(useg, USIZE, 1);
	regrele(c_prp->p_reg);

	return(0);
}

/*
 *	build a u area and address space for the child
 */
struct user *makechild(pp, cp)
register struct proc *pp, *cp;
{
	register pte_t	*ppt, *cpt;
	register int i;
	int	ptsize, nptaddr;
	pt_t	*seg;
	struct user *useg;
	label_t	tsav;

	/*
	 *	capture save area for child before entering memory
	 *	allocation routines, which may sleep, thus bashing
	 *	the current save area contents
	 */
	for (i=0; i<10; i++)
		tsav[i] = u.u_rsav[i];

	/*
	 *	Initially 1 page of page table space
	 */
	if ((seg = (pt_t *)sptalloc(1, 0, -1)) == 0)
		return(NULL);
	nptaddr = svtop(seg);
	cp->p_ptaddr = nptaddr;
	cp->p_spt = (int *)seg;
	cp->p_nspt = 1;
	cp->p_ptsize = 1;

	/*
	 * Allocate for U block
	 */
	if ((useg = (struct user *)sptalloc(USIZE, PG_V|PG_KW, 0)) == 0) {
		sptfree(seg, ptsize, 1);
		return(NULL);
	}

	/*
	 *	copy the u area
	 */
	
	ppt = (pte_t *)ublkptaddr(pp);
	cpt = (pte_t *)(&sbrpte[svtop(useg)]);
	for (i=USIZE; i-->0; ++ppt, ++cpt) {
		copypage(ppt->pgm.pg_pfn, cpt->pgm.pg_pfn);
		cpt->pgm.pg_prot = PTE_KW;
		cpt->pgm.pg_v = PT_VAL;
	}

	/*
	 *	adjust process context, update base and length registers
	 */

	for (i=0; i<10; i++)
		useg->u_rsav[i] = tsav[i];
	useg->u_pcb.pcb_p0br = (int) seg;
	useg->u_pcb.pcb_p0lr = 0x04000000;
	cp->p_p0size = 0;
	useg->u_pcb.pcb_p1br = (int)(&seg[1][-REGSZPGS]);
	useg->u_pcb.pcb_p1lr = REGSZPGS;
	cp->p_p1size = 0;
	useg->u_ssize = 0;
	useg->u_procp = cp;

	return(useg);
}


uaccess(spte)
register spte_t *spte;
{
	register i, v;

	v = (int)uservad;
	for (i=0; i<btoc(sizeof u); i++) {
		userpte[i] = *spte++;
		mtpr(TBIS, v + ctob(i));
	}
}
