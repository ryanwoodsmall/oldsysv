/* @(#)lock.c	1.1 */
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/lock.h"

char runlock;

lock()
{
	struct a {
		int oper;
	};
	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->oper) {
	case UNLOCK:
		if (u.u_lock&(TXTLOCK|DATLOCK)) {
			punlock();
			return;
		}
		goto bad;
	case PROCLOCK:
		if (u.u_lock&(TXTLOCK|DATLOCK))
			goto bad;
		textlock();
		datalock();
		break;
	case TXTLOCK:
		if ((u.u_lock&(TXTLOCK)) || (u.u_procp->p_textp==NULL))
			goto bad;
		textlock();
		break;
	case DATLOCK:
		if ((u.u_lock&(DATLOCK)) || (u.u_procp->p_textp==NULL))
			goto bad;
		datalock();
		break;
	default:
		goto bad;
	}
	if (runlock==0)
		return;
	if (runout) {
		runout = 0;
		wakeup((caddr_t)&runout);
	}
	sleep((caddr_t)&runlock,PZERO-1);
	return;

bad:
	u.u_error = EINVAL;
}

static
datalock()
{
	u.u_procp->p_flag |= SSYS;
	u.u_lock |= DATLOCK;
	runlock++;
}

static
textlock()
{
	struct text *xp;

	u.u_lock |= TXTLOCK;
	if ((xp=u.u_procp->p_textp)==NULL) return;
	if (xp->x_lcount++ == 0) {
		runlock++;
		xp->x_ccount++;
	}
}

punlock()
{
	struct text *xp;

	u.u_procp->p_flag &= ~SSYS;
	u.u_lock = 0;
	if (((xp=u.u_procp->p_textp) != NULL) && ( --(xp->x_lcount) == 0))
		xccdec(xp);
	runlock++;
}

shuffle()
{
	register struct proc *pp;
	register struct text *xp;

	runlock = 0;
	for(pp= &proc[1]; pp< v.ve_proc; pp++) {
		if (pp->p_stat == SIDL)
			continue;
		if ((pp->p_flag&SLOCK) || (pp->p_stat==NULL) || (pp->p_stat==SZOMB)
		  || ((xp=pp->p_textp) && xp->x_flag&XLOCK))
			continue;
		if (pp->p_flag&SLOAD) {
			/* swap out complete process */
			pp->p_flag &= ~SLOAD;
			xswap(pp,1,0);
		}
		if (xp != NULL && xp->x_lcount && xp->x_ccount==1)
			/*swap text out */
			xccdec(xp);
	}
	for(pp= &proc[1]; pp< v.ve_proc; pp++) {
		if (pp->p_stat == SIDL)
			continue;
		if ((pp->p_stat==NULL) || (pp->p_stat==SZOMB))
			continue;
		if ((xp=pp->p_textp) != NULL && xp->x_lcount
			&& xp->x_ccount == 0)
			/*swap text in only */
			xswapin(xp);
		if ((pp->p_flag&SSYS) && (pp->p_flag&SLOAD) == 0)
			/*swap data only in */
			swapin(pp);
	}
	wakeup((caddr_t)&runlock);
}
