/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/text.c	10.18"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"


/*	Allocate text region for a process
 */
struct inode *
xalloc(exec_data, tpregtyp)
register struct exdata *exec_data;
{
	register struct inode   *ip = exec_data->ip;
	register reg_t		*rp;
	register preg_t		*prp;
	register int		size = exec_data->ux_tsize;
	register caddr_t	org = (caddr_t)(exec_data->ux_toffset);
	register int		base = exec_data->ux_txtorg;
	register int		regva = base & ~SOFFMASK;
	extern short dufstyp;

	if (size == 0)
		return(ip);
	/* kind a KLUDGY but reduces number of DU hooks
	 * if the ip if remote overwrite magic number to 410
	 */
	if (ip->i_fstyp == dufstyp) 
		exec_data->ux_mag = 0410;

	/*	Search the region table for the text we are
	 *	looking for.
	 */

loop:
	rlstlock();

	for (rp = ractive.r_forw ; rp != &ractive ; rp = rp->r_forw) {
		if (rp->r_type == RT_STEXT  &&
		   rp->r_iptr == ip  &&
		   (rp->r_flags & RG_NOSHARE) == 0) {
			rlstunlock();
			reglock(rp);
			if (rp->r_type != RT_STEXT || rp->r_iptr != ip) {
				regrele(rp);
				goto loop;
			}
			/*	Artificially bump the reference count
			 *	to make sure the region doesn't go away
			 *	while we are waiting.
			 *	Since we have the inode locked, we should
			 *	never get into xalloc with RG_DONE not set.
			 */
			if ((rp->r_flags & RG_DONE) == 0) {
				rp->r_refcnt++;
				rp->r_waitcnt++;
				regrele(rp);
				regwait(rp);
				reglock(rp);
				rp->r_refcnt--;
			}
			prp = attachreg(rp, &u, regva, tpregtyp, SEG_RO);
			regrele(rp);
			if (prp == NULL) {
				iput(ip);
				return(NULL);
			}
			return(ip);
		}
	}
	rlstunlock();
	
	/*	Text not currently being executed.  Must allocate
	 *	a new region for it.
	 *	Inode is locked by getxfile;  it expects us to
	 *	do an iput on every failure.  Since allocreg bumps
	 *	ip->i_count, we must decrement here.  Don't set
	 *	RG_NOFREE until the end also.
	 */

	if ((rp = allocreg(ip, RT_STEXT, 0)) == NULL) {
		iput(ip);
		return(NULL);
	}

	/*	Attach the region to our process.
	 */
	
	if ((prp = attachreg(rp, &u, regva, tpregtyp, SEG_RW)) == NULL) {
		ip->i_count--;
		freereg(rp);
		return(NULL);
	}
	
	/*	Load the region or map it for demand load.
	 */

	switch (exec_data->ux_mag) {
	case 0413:
	case 0443:
		if (mapreg(prp, base, ip, org, size) < 0) {
			ip->i_count--;
			detachreg(prp, &u);
			return(NULL);
		}
		break;


		/* fall thru case if REMOTE */
	case 0410:
		if (loadreg(prp, base, ip, org, size) < 0) {
			ip->i_count--;
			detachreg(prp, &u);
			return(NULL);
		}
		break;
	default:
		cmn_err(CE_PANIC, "xalloc - bad magic");
	}

	/*	The pipelining cpu does instruction prefetch.
	**	There is a problem that the 32000 chip tells
	**	the mmu that it is doing a normal instruction
	**	fetch so that if the address is not valid,
	**	an external memory fault occurs.  Since the
	**	address is beyond the end of the text space,
	**	this will result in giving the process a SIGSEGV.
	**	The fix in the hardware is to never give an
	**	external fault for an instruction prefetch and
	**	for the cpu to put the correct access type on
	**	the bus to the mmu. In order for paging to run
	**	on the old hardware we must grow the text by a
	**	page if it ends near the end of a page.
	*/

	if(btoc(size + PREFETCH) > btoc(size)){
		if(growreg(prp, 1, DBD_DZERO) < 0){
			ip->i_count--;
			detachreg(prp, &u);
			return(NULL);
		}
	}

	if (!FS_ACCESS(ip, ISVTX))
		rp->r_flags |= RG_NOFREE;

	ASSERT(prp->p_reg->r_listsz != 0);
	chgprot(prp, SEG_RO);
	regrele(rp);
	return(ip);
}


/*	Free the swap image of all unused shared text regions
 *	which are from device dev (used by umount system call).
 */

xumount(mp)
register struct mount *mp;
{
	register reg_t		*rp;
	register reg_t		*nrp;
	register int		count;
	register inode_t	*ip;

	count = 0;
loop:
	rlstlock();

	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		if (rp->r_type != RT_STEXT)
			nrp = rp->r_forw;
		else {
			rlstunlock();
			/*
			 * There is a race here.  At this point the region
			 * is not locked, and while we sleep waiting for
			 * the inode lock, the region may be freed and the
			 * inode count may drop to zero; when we finally
			 * get the inode we panic on the assertion in plock()
			 * which says that the count must not be zero.
			 *
			 * The correct fix is to lock the region before locking
			 * the inode, but to avoid deadlocks it has to be done
			 * in the same order throughout the system.  Next
			 * release, guys.  Workaround for the moment: put the
			 * code for plock() and prele() inline here, without
			 * the i_count assertions.  (I don't like it, either.)
			 */
			if (ip = rp->r_iptr) {
				/* Temporary: plock(ip) */
#ifdef DEBUG
				ilog(ip, 3, xumount);
#endif
				while (ip->i_flag & ILOCK) {
					ip->i_flag |= IWANT;
					sleep((caddr_t)ip, PINOD);
				}
				ip->i_flag |= ILOCK;
				ASSERT(addilock(ip) == 0);
			}
			reglock(rp);
			if (rp->r_type != RT_STEXT) {
				regrele(rp);
				if (ip) {
					/* Temporary: prele(ip) */
#ifdef DEBUG
					ilog(ip, 4, xumount);
#endif
					ASSERT(ip->i_flag & ILOCK);
					ip->i_flag &= ~ILOCK;
					if (ip->i_flag & IWANT) {
						ip->i_flag &= ~IWANT;
						wakeup((caddr_t)ip);
					}
					ASSERT(rmilock(ip) == 0);
				}
				goto loop;
			}
			rlstlock();
			nrp = rp->r_forw;
			if (mp == ip->i_mntdev && rp->r_refcnt == 0) {
				rlstunlock();
				freereg(rp);
				count++;
				goto loop;
			} else {
				regrele(rp);
				if (ip)
					prele(ip);
			}
		}
	}
	rlstunlock();
	return(count);
}
/*	Remove a shared text region associated with inode ip from
 *	the region table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register reg_t	*rp;
	register reg_t	*nrp;

	ASSERT(ip->i_flag & ILOCK);
	if ((ip->i_flag&ITEXT) == 0)
		return;
	
loop:
	rlstlock();

	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		if (rp->r_type != RT_STEXT || ip != rp->r_iptr)
			nrp = rp->r_forw;
		else {
			rlstunlock();
			reglock(rp);
			if (rp->r_type != RT_STEXT || ip != rp->r_iptr) {
				regrele(rp);
				goto loop;
			}
			rlstlock();
			nrp = rp->r_forw;
			if (rp->r_refcnt == 0) {
				rlstunlock();
				freereg(rp);
				plock(ip);
				goto loop;
			} else {
				regrele(rp);
			}
		}
	}
	rlstunlock();
}


/*	Try to removed unused sticky regions in order to free up swap
 *	space.
 */

swapclup()
{
	register reg_t		*rp;
	register reg_t		*nrp;
	register int		rval;
	register inode_t	*ip;

	rval = 0;

loop:
	rlstlock();

	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		nrp = rp->r_forw;
		if (rp->r_type == RT_SHMEM)
			continue;
		if (ip = rp->r_iptr) {
			if (ip->i_flag & ILOCK)
				continue;
			plock(ip);
		}
		if (rp->r_flags & RG_LOCK) {
			if (ip)
				prele(ip);
			continue;
		}
		reglock(rp);
		if (rp->r_type == RT_UNUSED) {
			regrele(rp);
			if (ip)
				prele(ip);
			continue;
		}
		if (rp->r_refcnt == 0) {
			rlstunlock();
			freereg(rp);
			rval = 1;
			goto loop;
		} else {
			regrele(rp);
			if (ip)
				prele(ip);
		}
	}
	rlstunlock();
	return(rval);
}

#ifndef vax

#define FORCE	1

/*
 * Prepare object process text region for writing.  If there is only
 * one reference to the region, and it's not sticky (RG_NOFREE is not
 * set), change the region type to private (instead of shared-text)
 * to prevent it from being found by any subsequent text searches.
 * Otherwise, make a private copy-on-write duplicate, detach the
 * original region from the object process, and attach the duplicate
 * in its place.  The pregion pointer of the new (private) text region
 * is returned.
 *
 * The original region and the associated inode (if any) must be locked
 * before entry to xdup(); on return the new region will have been
 * locked and the inode will be (still) locked.  On error, xdup()
 * returns NULL and unlocks both the region and the inode.
 */
preg_t *
xdup(p, prp)
register struct proc *p;
register preg_t *prp;
{
	register reg_t *rp = prp->p_reg, *nrp;
	register struct inode *ip;
	register preg_t *nprp;
	caddr_t va;
	register user_t *up;

	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(rp->r_type == RT_STEXT);
	if (rp->r_refcnt == 1 && (rp->r_flags & RG_NOFREE) == 0) {
		nprp = prp;
		rp->r_type = RT_PRIVATE;
	} else {
		ip = rp->r_iptr;
		va = prp->p_regva;
		if ((nrp = dupreg(rp, NOSLEEP, FORCE)) == NULL) {
			regrele(rp);
			return(NULL);
		}
		ip = nrp->r_iptr;
		winublock();
		up = (user_t *)win_ublk;
		kvtokstbl(up)->wd2.address = kvtophys(ubptbl(p));
		flushmmu(up, USIZE);
		detachreg(prp, up);
		/* detachreg() unlocks ip */
		if (ip)
			plock(ip);
		if ((nprp = attachreg(nrp, up, va, PT_TEXT, SEG_RO)) == NULL
		  && ip)
			prele(ip);
		winubunlock();
	}
	return(nprp);
}

#else

/*
 * Prepare object process text region for writing.  If there is currently
 * only one reference to it, and it's not sticky text (RG_NOFREE set on the
 * region), remove it from the active region list so that it will no longer
 * be found by xalloc() and change the region type (not the pregion type)
 * to RT_PRIVATE.  If more than one process is using it, make a private
 * duplicate, detach the original region from the object process, and attach
 * the duplicate in its place.  In either case return the pregion pointer
 * of the (new or old, and now private) text region.
 */
preg_t *
xdup(p, prp)
register struct proc *p;
register preg_t *prp;
{
	register reg_t *rp = prp->p_reg, *nrp;
	register preg_t *nprp;
	caddr_t va;

	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(rp->r_type == RT_STEXT);
	if (rp->r_refcnt == 1 && (rp->r_flags & RG_NOFREE) == 0) {
		nprp = prp;
		/* Remove it from the active region list */
		rp->r_back->r_forw = rp->r_forw;
		rp->r_forw->r_back = rp->r_back;
		rp->r_type = RT_PRIVATE;
	} else {
		extern struct user *uservad;
		va = prp->p_regva;
		if ((nrp = dupreg(rp, SEG_RO, 1)) == NULL) {
			regrele(rp);
			return(NULL);
		}
		/* Should RG_DONE be set on the new region? */
		uaccess(&(baseseg[p->p_ptaddr+p->p_ptsize][-USIZE]));
		detachreg(uservad, prp);
		nprp = attachreg(nrp, uservad, va, PT_TEXT, SEG_RO);
	}
	return(nprp);
}

#endif
