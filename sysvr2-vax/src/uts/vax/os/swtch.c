/* @(#)swtch.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/page.h"
#include <sys/region.h>
#include "sys/proc.h"
#include "sys/mtpr.h"
#include "sys/sysinfo.h"

struct proc *lastproc;
extern int switching;

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
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */
swtch()
{
	register n;
	register struct proc *p, *q, *pp, *pq;
	preg_t *prp;
	extern struct user *uservad;

	sysinfo.pswitch++;
	if (save(u.u_rsav)) {
		p = lastproc;
		switch(p->p_stat) {
		case SZOMB:
			if ((prp = findpreg(p, PT_STACK)) == NULL) {
				panic("swtch: findpreg");
			}
			reglock(prp->p_reg);
			uaccess(&(baseseg[p->p_ptaddr+p->p_ptsize][-USIZE]) );
			detachreg(uservad, prp);
			sptfree(vtop(p->p_ptaddr, 0), p->p_ptsize, 0);
			break;
		}
		return;
	}

	switching = 1;
	lastproc = u.u_procp;
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
	 * The rsav contents are interpreted in the new address space
	 */
	switching = 0;
	p->p_flag &= ~SSWAP;
	resume(p->p_addr, u.u_rsav);
}
