/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/disp.c	10.4"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/cmn_err.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/inline.h"

struct proc *disp()
{
	register n;
	register struct proc *p, *q, *pp, *pq;
	extern int slice_size;
	extern int lticks;

	pp = NULL;
	q = NULL;
	n = 128;

	for (p = runq  ;  p  ;  q = p, p = p->p_link) {
		ASSERT(p->p_stat == SRUN);
		if (p->p_pri > n)
			continue;
		if ((p->p_flag & (SLOAD|SPROCIO)) != SLOAD)
			continue;
		pp = p;
		pq = q;
		n = p->p_pri;
	}

	/*
	 * If no process is runnable, idle.
	 */

	if(!(p = pp))
		return(0);
	q = pq;

	/* Take p off the runq */

	if (q == NULL)
		runq = p->p_link;
	else
		q->p_link = p->p_link;

	ASSERT(p->p_stat == SRUN  &&  (p->p_flag & SLOAD));

	p->p_stat = SONPROC;	/* process p will be running	*/
	lticks = slice_size;	/* allocate process a slice  */
	curpri = n;
	curproc = p;
	return(p);
	
}	


setrq(p)
register struct proc *p;
{
	register struct proc *q;
	register s;

	s = splhi();
	ASSERT(p->p_stat == SRUN || p->p_stat == SONPROC);
	for (q=runq; q!=NULL; q=q->p_link)
		if (q == p) {
			cmn_err(CE_PANIC, "setrq - proc on q.");
			goto out;
		}
	p->p_link = runq;
	runq = p;
out:
	splx(s);
}



chgpri(pnice, nice)
int pnice, nice;
{
	
	if ((nice < 0 || nice > 2*NZERO) && !suser())
		nice = 0;
	nice += pnice;
	if (nice >= 2*NZERO)
		nice = 2*NZERO -1;
	if (nice < 0)
		nice = 0;
	return(nice);
}
