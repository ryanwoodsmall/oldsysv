/* @(#)slp.c	1.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/lock.h"

#define	NHSQUE	16	/* must be power of 2 */
#define	sqhash(X)	(&hsque[((int)X >> 3) & (NHSQUE-1)])
struct proc *hsque[NHSQUE];
char	runin, runout, runrun, curpri;
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
#define	MAXCOPY	128
sleep(chan, disp)
caddr_t chan;
{
	register struct proc *rp = u.u_procp;
	register struct proc **q = sqhash(chan);
	register s;

	s = spl6();
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
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
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
	register s;


	s = spl6();
	for (q = sqhash(chan); p = *q; )
		if (p->p_wchan==chan && p->p_stat==SSLEEP) {
			p->p_stat = SRUN;
			p->p_wchan = 0;
			/* take off sleep queue, put on run queue */
			*q = p->p_link;
			p->p_link = runq;
			runq = p;
			if (!(p->p_flag&SLOAD)) {
				p->p_time = 0;
				/* defer setrun to avoid breaking link chain */
				if (runout > 0)
					runout = -runout;
			} else if (p->p_pri < curpri)
				runrun++;
		} else
			q = &p->p_link;
	if (runout < 0) {
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

	s = spl6();
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

	s = spl6();
	if (p->p_stat == SSLEEP) {
		/* take off sleep queue */
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link) ;
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
		if (runout > 0) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else if (p->p_pri < curpri)
		runrun++;
	splx(s);
}

/*
 * The main loop of the scheduling (swapping) process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * The runout flag is set whenever someone is swapped out.
 * Sched sleeps on it awaiting work.
 *
 * Sched sleeps on runin whenever it cannot find enough
 * memory (by swapping out or otherwise) to fit the
 * selected swapped process.  It is awakened when the
 * memory situation changes and in any case once per second.
 */
sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	int maxbad;
	int tmp;

	/*
	 * find user to swap in;
	 * of users ready, select one out longest
	 */

loop:
	spl6();
	if (runlock)
		shuffle();
	outage = -20000;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++)
	if (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0 &&
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
	 * if so, swap it in.
	 */

	if (swapin(p))
		goto loop;

	/*
	 * none found.
	 * look around for memory.
	 * Select the largest of those sleeping
	 * at bad priority; if none, select the oldest.
	 */

	spl6();
	p = NULL;
	maxbad = 0;
	inage = 0;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if (rp->p_stat==SZOMB
		 || (rp->p_flag&(SSYS|SLOCK|SLOAD))!=SLOAD)
			continue;
		if (rp->p_textp && rp->p_textp->x_flag&XLOCK)
			continue;
		if (rp->p_stat==SSLEEP || rp->p_stat==SSTOP) {
			tmp = rp->p_pri - PZERO + rp->p_time;
			if (maxbad < tmp) {
				p = rp;
				maxbad = tmp;
			}
		} else if (maxbad<=0 && rp->p_stat==SRUN) {
			tmp = rp->p_time + rp->p_nice - NZERO;
			if (tmp > inage) {
				p = rp;
				inage = tmp;
			}
		}
	}
	spl0();
	/*
	 * Swap found user out if sleeping at bad pri,
	 * or if he has spent at least 2 seconds in memory and
	 * the swapped-out process has spent at least 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */
	if (maxbad>0 || (outage>=2 && inage>=2)) {
		p->p_flag &= ~SLOAD;
		xswap(p, 1, 0);
		goto loop;
	}
	spl6();
	runin++;
	sleep((caddr_t)&runin, PSWP);
	goto loop;
}

/*
 * Swap a process in.
 * Allocate data and possible text separately.
 * It would be better to do largest first.
 */
swapin(p)
register struct proc *p;
{
	register struct text *xp;
	register ushort a;
	ushort x;

	if ((a = malloc(coremap, p->p_size)) == NULL)
		return(0);
	if (xp = p->p_textp) {
		if (xswapin(xp) == 0) {
			mfree(coremap, p->p_size, a);
			return(0);
		}
	}
	swap(p->p_addr, a, p->p_size, B_READ);
	mfree(swapmap, ctod(p->p_size), p->p_addr);
	p->p_addr = a;
	p->p_flag |= SLOAD;
	p->p_time = 0;
	return(1);
}

xswapin(xp)
register struct text *xp;
{
	register x;

	xlock(xp);
	if (xp->x_ccount == 0) {
		if ((x = malloc(coremap, xp->x_size)) == NULL) {
			xunlock(xp);
			return(0);
		}
		xp->x_caddr = x;
		if ((xp->x_flag&XLOAD) == 0)
			swap(xp->x_daddr,x,xp->x_size,B_READ);
	}
	xp->x_ccount++;
	xunlock(xp);
	return(1);
}

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler.
 */
qswtch()
{

	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 */
swtch()
{
	register n;
	register struct proc *p, *q, *pp, *pq;

	/*
	 * If not the idle process, resume the idle process.
	 */
	sysinfo.pswitch++;
	if (u.u_procp != &proc[0]) {
		if (u.u_procp->p_flag&SLOAD) {
			if (u.u_fpsaved == 0) {
				savfp(&u.u_fps);
				u.u_fpsaved = 1;
			}
			if (save(u.u_rsav)) {
				sureg();
				return;
			}
		}
		resume(proc[0].p_addr, u.u_qsav);
	}
	/*
	 * The first save returns nonzero when proc 0 is resumed
	 * by another process (above); then the second is not done
	 * and the process-search loop is entered.
	 *
	 * The first save returns 0 when swtch is called in proc 0
	 * from sched().  The second save returns 0 immediately, so
	 * in this case too the process-search loop is entered.
	 * Thus when proc 0 is awakened by being made runnable, it will
	 * find itself and resume itself at rsav, and return to sched().
	 */
	if (save(u.u_qsav) == 0 && save(u.u_rsav))
		return;
loop:
	spl6();
	runrun = 0;
	pp = NULL;
	q = NULL;
	n = 128;
	/*
	 * Search for highest-priority runnable process
	 */
	if (p = runq) do {
		if ((p->p_flag&SLOAD) && p->p_pri <= n) {
			pp = p;
			pq = q;
			n = p->p_pri;
		}
		q = p;
	} while (p = p->p_link);
	/*
	 * If no process is runnable, idle.
	 */
	p = pp;
	if (p == NULL) {
		curpri = PIDLE;
		curproc = &proc[0];
		idle();
		goto loop;
	}
	q = pq;
	if (q == NULL)
		runq = p->p_link;
	else
		q->p_link = p->p_link;
	curpri = n;
	curproc = p;
	spl0();
	/*
	 * The rsav (ssav) contents are interpreted in the new address space
	 */
	n = p->p_flag&SSWAP;
	p->p_flag &= ~SSWAP;
	resume(p->p_addr, n? u.u_ssav: u.u_rsav);
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc()
{
	register struct proc *rpp, *rip;
	register n;
	struct proc *pend;
	static mpid;

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
	rpp = NULL;
retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	rip = &proc[0];
	n = v.v_proc;
	do {
		if (rip->p_stat == NULL) {
			if (rpp == NULL)
				rpp = rip;
		} else
			pend = rip;
		if (rip->p_pid==mpid)
			goto retry;
		rip++;
	} while(--n);
	if (rpp==NULL)
		panic("no procs");
	if (rpp > pend)
		pend = rpp;
	pend++;
	v.ve_proc = (char *)pend;

	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	rpp->p_stat = SRUN;
	rpp->p_clktim = 0;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_suid = rip->p_suid;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = 0;
	rpp->p_cpu = rip->p_cpu;
	rpp->p_pri = PUSER + rip->p_nice - NZERO;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if (u.u_ofile[n] != NULL)
			u.u_ofile[n]->f_count++;
	if (rip->p_textp != NULL) {
		rip->p_textp->x_count++;
		rip->p_textp->x_ccount++;
	}
	u.u_cdir->i_count++;
	if (u.u_rdir)
		u.u_rdir->i_count++;
	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	u.u_procp = rpp;
	rpp->p_size = rip->p_size;
	rpp->p_addr = rip->p_addr;
	if (u.u_fpsaved == 0) {
		savfp(&u.u_fps);
		u.u_fpsaved = 1;
	}
	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(u.u_ssav)) {
		sureg();
		return(1);
	}
	/*
	 * If there is not enough memory for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if ((rip->p_size > MAXCOPY) || procdup(rpp) == NULL) {
		rip->p_stat = SIDL;
		xswap(rpp, 0, 0);
		rip->p_stat = SRUN;
	}
	u.u_procp = rip;
	setrq(rpp);
	rpp->p_flag |= SSWAP;
	return(0);
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release the extra memory.
 * If it's growing, and there is memory, just allocate it
 * and copy the image, taking care to reset registers to account
 * for the fact that the system's stack has moved.
 * If there is no memory, arrange for the process to be swapped
 * out after adjusting the size requirement-- when it comes
 * in, enough memory will be allocated.
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
expand(newsize)
ushort newsize;
{
	register i;
	register ushort n;
	register struct proc *p;
	register ushort a1, a2;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	a1 = p->p_addr;
	if (n >= newsize) {
		mfree(coremap, n-newsize, a1+newsize);
		return;
	}
	if (u.u_fpsaved == 0) {
		savfp(&u.u_fps);
		u.u_fpsaved = 1;
	}
	if (save(u.u_ssav)) {
		sureg();
		return;
	}
	if ((n > MAXCOPY) || (a2 = malloc(coremap, newsize)) == NULL) {
		xswap(p, 1, n);
		p->p_flag |= SSWAP;
		if (u.u_lock&(DATLOCK))
			runlock++;
		qswtch();
		/* no return */
	}
	for(i=0; i<n; i++)
		copyseg(a1+i, a2+i);
	p->p_addr = a2;
	mfree(coremap, n, a1);
	if (u.u_lock&(DATLOCK))
		runlock++;
	resume(a2, u.u_ssav);
}
