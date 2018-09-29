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
/* @(#)text.c	6.5 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/inode.h>
#include <sys/buf.h>
#include <sys/var.h>
#include <sys/sysinfo.h>
#include <sys/seg.h>
#include <sys/page.h>
#include <sys/pfdat.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/debug.h>

/*
 * Allocate text region for a process
 */
xalloc(ip)
register struct inode *ip;
{
	register reg_t		*rp;
	register preg_t		*prp;
	register int		size;
	register caddr_t	org;

	if((size = u.u_exdata.ux_tsize) == 0)
		return(0);

	/*	Search the region table for the text we are
	 *	looking for.
	 */

loop:
	for(rp = ractive.r_forw ; rp != &ractive ; rp = rp->r_forw){
		if(rp->r_iptr == ip  &&  rp->r_type == RT_STEXT){
			reglock(rp);
			if(rp->r_iptr != ip  ||  rp->r_type != RT_STEXT){
				regrele(rp);
				goto loop;
			}
			while(!(rp->r_flags & RG_DONE)){
				rp->r_flags |= RG_WAITING;
				regrele(rp);
				sleep(&rp->r_flags, PZERO);
				reglock(rp);
			}
			prp = attachreg(rp, &u,  0, PT_TEXT, SEG_RO);
			regrele(rp);
			if(prp == NULL)
				return(-1);
			return((int)prp);
		}
	}
	
	/*	Text not currently being executed.  Must allocate
	 *	a new region for it.
	 */

	if((rp = allocreg(ip, RT_STEXT)) == NULL)
		return(-1);

	if(ip->i_mode & ISVTX)
		rp->r_flags |= RG_NOFREE;
	org = (caddr_t)u.u_exdata.ux_txtorg;
	
	/*	Attach the region to our process.
	 */
	
	if((prp = attachreg(rp, &u, 0, PT_TEXT, SEG_RO)) == NULL){
		freereg(rp);
		return(-1);
	}
	
	/*	Load the region or map it for demand load.
	 */

	if(u.u_exdata.ux_mag == 0413){
/*
		ASSERT(u.u_exdata.ux_tstart == 0);
*/
		if(mapreg(prp, org, ip, u.u_exdata.ux_tstart, size) < 0){
			detachreg(&u, prp);
			return(-1);
		}
	}else
		panic("xalloc - bad magic");

	chgprot(prp, SEG_RO);
	regrele(rp);
	return(0);
}





/*
 * free the swap image of all unused saved-text text segments
 * which are from device dev (used by umount system call).
 */
xumount(dev)
register dev_t dev;
{
	register struct inode	*ip;
	register reg_t		*rp;
	register reg_t		*nrp;
	register		count;

	while(1){
		count = 0;
		for(rp = ractive.r_forw ; rp != &ractive ; rp = nrp){
			reglock(rp);
			if(rp->r_type == RT_UNUSED){
				regrele(rp);
				break;
			}
			nrp = rp->r_forw;
			if ((ip = rp->r_iptr) == NULL){
				regrele(rp);
				continue;
			}
			if (dev != NODEV && dev != ip->i_dev){
				regrele(rp);
				continue;
			}
			if (xuntext(rp))
				count++;
			else
				regrele(rp);
		}
		if(rp == &ractive)
			return(count);
	}
}

/*
 * remove a shared text segment from the region table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register reg_t	*rp;
	register reg_t	*nrp;

	if ((ip->i_flag&ITEXT) == 0)
		return;
	
	while(1){
		for(rp = ractive.r_forw ; rp != &ractive ; rp = nrp){
			reglock(rp);
			if(rp->r_type == RT_UNUSED){
				regrele(rp);
				break;
			}
			nrp = rp->r_forw;
			if (ip == rp->r_iptr){
				if(!xuntext(rp))
					regrele(rp);
			} else {
				regrele(rp);
			}
		}
		if(rp == &ractive)
			return;
	}
}


/*	Try to removed unused regions in order to free up swap
 *	space.
 */

swapclup()
{
	register reg_t	*rp;
	register reg_t	*nrp;
	register int	rval;

	while(1){
		rval = 0;
		for(rp = ractive.r_forw ; rp != &ractive ; rp = nrp){
			reglock(rp);
			if(rp->r_type == RT_UNUSED){
				regrele(rp);
				break;
			}
			nrp = rp->r_forw;
			if(!xuntext(rp))
				regrele(rp);
			else
				rval = 1;
		}
		if(rp == &ractive)
			return(rval);
	}
}

/*
 * remove text image from the region table.
 * the use count must be zero.
 */
xuntext(rp)
register reg_t	*rp;
{
	if (rp->r_refcnt != 0)
		return(0);
	freereg(rp);
	return(1);
}
