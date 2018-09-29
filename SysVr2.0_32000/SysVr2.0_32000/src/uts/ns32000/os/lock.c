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
/* @(#)lock.c	6.3 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/lock.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/proc.h>

lock()
{
	struct a {
		long oper;
	};

	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->oper) {
	case TXTLOCK:
		if ((u.u_lock&(PROCLOCK|TXTLOCK)) || textlock() == 0)
			goto bad;
		break;
	case PROCLOCK:
		if ((u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK))  ||
		    datalock()  == 0  ||
		    proclock() == 0  )
			goto bad;
		(void) textlock();
		break;
	case DATLOCK:
		if ((u.u_lock&(PROCLOCK|DATLOCK))  ||  datalock() == 0)
			goto bad;
		break;
	case UNLOCK:
		if (punlock() == 0)
			goto bad;
		break;

	default:
bad:
		u.u_error = EINVAL;
	}
}

textlock()
{
	register preg_t	*prp;
	register reg_t	*rp;


	prp = findpreg(u.u_procp, PT_TEXT);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	rp->r_flags |= RG_NOSWAP;
	u.u_lock |= TXTLOCK;
	return(1);
}
		
tunlock()
{
	register preg_t	*prp;
	register reg_t	*rp;

	prp = findpreg(u.u_procp, PT_TEXT);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	rp->r_flags &= ~RG_NOSWAP;
	u.u_lock &= ~TXTLOCK;
	return(1);
}

datalock()
{
	register preg_t	*prp;
	register reg_t	*rp;


	prp = findpreg(u.u_procp, PT_DATA);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	rp->r_flags |= RG_NOSWAP;
	
	prp = findpreg(u.u_procp, PT_STACK);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	rp->r_flags |= RG_NOSWAP;
	u.u_lock |= DATLOCK;
	return(1);
}
		
dunlock()
{
	register preg_t	*prp;
	register reg_t	*rp;


			
	prp = findpreg(u.u_procp, PT_DATA);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	rp->r_flags &= ~RG_NOSWAP;
	u.u_lock &= ~DATLOCK;
	return(1);
}

proclock()
{
	u.u_procp->p_flag |= SSYS;
	u.u_lock |= PROCLOCK;
	return(1);
}

punlock()
{
	if ((u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK)) == 0)
		return(0);
	u.u_procp->p_flag &= ~SSYS;
	u.u_lock &= ~PROCLOCK;
	tunlock();
	dunlock();
	return(1);
}
