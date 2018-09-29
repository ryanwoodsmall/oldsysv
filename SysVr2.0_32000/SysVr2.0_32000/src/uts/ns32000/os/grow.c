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
/* @(#)grow.c	1.3 */
#include <sys/types.h>
#include <sys/tuneable.h>
#include <sys/bitmasks.h>
#include <sys/param.h>
#include <sys/mmu.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/var.h>
#include <sys/page.h>
#include <sys/pfdat.h>
#include <sys/seg.h>
#include <sys/region.h>
#include <sys/proc.h>

#include <sys/debug.h>

/*
 * brk and sbrk system calls
 */
sbreak()
{
	struct a {
		int nva;
	};
	register preg_t	*prp;
	register reg_t	*rp;
	register int	nva;
	register int	change;
	register int	n;

	/*	Find the processes data region.
	 */

	prp = findpreg(u.u_procp, PT_DATA);
	if(prp == NULL)
		return;
	rp = prp->p_reg;
	reglock(rp);

	nva = ((struct a *)u.u_ap)->nva;
	if (nva < u.u_datorg)  {
		nva = u.u_datorg;
	}

	if ((change = btop((int)nva) - btotp(prp->p_regva)
			- (rp->r_pgsz + rp->r_pgoff) ) == 0) {
		regrele(rp);
		return;
	}

	ASSERT(rp->r_pgsz+change >= 0);
	if(growreg(&u, prp, change, 0, DBD_DZERO) < 0){
		regrele(rp);
		return;
	}
	regrele(rp);

	if(change < 0)
		clratb(USRATB);
}

/*
 * grow the stack to include the SP
 * true return if successful.
 */
grow(sp)
unsigned sp;
{
	register preg_t	*prp;
	register reg_t	*rp;
	register	si;

	/*	Find the processes stack region.
	 */

	prp = findpreg(u.u_procp, PT_STACK);
	if(prp == NULL) {
		return(0);
	}
	rp = prp->p_reg;

	reglock(rp);

	if (sp >= (uint)USRSTACK - ptob(rp->r_pgsz)) {
		regrele(rp);
		return(0);
	}

	si = btop((uint)USRSTACK - sp) - (rp->r_pgsz) + SINCR;
	if (si <= 0){
		regrele(rp);
		return(0);
	}

	if(growreg(&u, prp, si, 0, DBD_DZERO) < 0){
		regrele(rp);
		return(0);
	}

	regrele(rp);
	return(1);
}
