/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/region.c	10.16"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"
#include "sys/message.h"
#include "sys/sysinfo.h"
#include "sys/conf.h"

reg_t	nullregion;
preg_t	nullpregion;
int	rlist_lock;

/*	The following defines the minimum number of
**	regions for a process.  This includes one
**	region each for the text, data, and stack
**	plus one for a dmm region,
**	plus one null region pointer to indicate
**	the end of the pregion list.
*/

#define	MINPREGPP	(3 + 1 + 1)


void
reginit()
{
	register reg_t		*rp;
	extern struct shminfo	shminfo;

	pregpp = MINPREGPP + shmseg() + 2*shlbinfo.shlbs;

	rfree.r_forw = &rfree;
	rfree.r_back = &rfree;

	ractive.r_forw = &ractive;
	ractive.r_back = &ractive;

	for (rp = region; rp < &region[v.v_region]; rp++) {
		rp->r_back = rfree.r_back;
		rp->r_forw = &rfree;
		rfree.r_back->r_forw = rp;
		rfree.r_back = rp;
	}
}

reglock(rp)
register reg_t *rp;
{
	while (rp->r_flags & RG_LOCK) {
		rp->r_flags |= RG_WANTED;
		sleep((caddr_t)rp, PZERO);
	}
	rp->r_flags |= RG_LOCK;
}

regrele(rp)
register reg_t *rp;
{
	ASSERT(rp->r_flags & RG_LOCK);
	rp->r_flags &= ~RG_LOCK;
	if (rp->r_flags & RG_WANTED) {
		rp->r_flags &= ~RG_WANTED;
		wakeup((caddr_t)rp);
	}
}

rlstlock()
{
	while (rlist_lock)
		sleep(&rlist_lock, PZERO);
	rlist_lock++;
}

rlstunlock()
{
	ASSERT(rlist_lock);
	rlist_lock = 0;
	wakeup(&rlist_lock);
}

regwait(rp)
register reg_t	*rp;
{
	while (!(rp->r_flags & RG_DONE))
	{	rp->r_flags |= RG_WAITING;
		sleep((caddr_t)rp->r_flags, PZERO);
	}
}


/*
 * Allocate a new region.
 * Returns a locked region pointer or NULL on failure
 * The region is linked into the active list.
 */

reg_t *
allocreg(ip, type, flag)
register struct inode	*ip;
short type;
short flag;
{
	register reg_t *rp;

	rlstlock();

	if ((rp = rfree.r_forw) == &rfree) {
		rlstunlock();
		cmn_err(CE_WARN, "Region table overflow");
		u.u_error = EAGAIN;
		return(NULL);
	}
	/*
	 * Remove from free list
	 */
	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	/* Initialize region fields and bump inode reference
	 * count.  Inode is locked by the caller.
	 */

	rp->r_type = type;
	rp->r_iptr = ip;
	rp->r_flags |= flag;
	reglock(rp);

	if (ip != NULL) {
		ASSERT(ip->i_flag & ILOCK);
		ip->i_count++;
	}

	/*
	 * Link onto active list
	 */
	rp->r_forw = ractive.r_forw;
	rp->r_back = &ractive;
	ractive.r_forw->r_back = rp;
	ractive.r_forw = rp;

	rlstunlock();
	return(rp);
}

/*
 * Free an unused region table entry.
 */
void
freereg(rp)
register reg_t *rp;	/* pointer to a locked region */
{
	register inode_t	*ip;
	register int		i;
	register int		lim;
	register int		size;
	register pde_t		*pt;
	register int		tsize;

	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(rp->r_refcnt == 0);

	/* If the region is still in use, then don't free it.
	 */

	ip = rp->r_iptr;

	if (rp->r_flags & RG_WANTED) {
		if (ip)
			prele(ip);
		regrele(rp);
		return;
	}

	/*
	 * Decrement use count on associated inode
	 * Inode is locked by the caller.
	 */

	if (ip) {
		ASSERT(ip->i_flag & ILOCK);
		iput(ip);
	}

	/*	Free the memory pages and the page tables and
	 *	disk use maps.  These latter are always allocated
	 *	together in pairs in a contiguous 128 word piece
	 *	of kernel virtual address space.  Note that the
	 *	pfree for the first page table is special because
	 *	pages may not have been allocated from the beginning
	 *	of the segment.  The last page table is also special
	 *	since it may not have been fully allocated.
	 */
	
	tsize = rp->r_pgsz;
	lim = ctos(tsize);

	for (i = 0  ;  i < lim  ;  i++) {
		pt = rp->r_list[i];
		size = tsize - stoc(i);
		if (size > NPGPT)
			size = NPGPT;
		memlock();
		pfree(rp, pt, pt + NPGPT, size);
		memunlock();
		uptfree(rp->r_list[i], 2);
	}
	availsmem += tsize;

	ASSERT(rp->r_noswapcnt >= 0  &&  rp->r_noswapcnt <= 1);
	if (rp->r_noswapcnt)
		availrmem += tsize;

	/*
	 * Free the list.
	 */
	
	uptfree(rp->r_list, rp->r_listsz);

	/*
	 * Remove from active list
	 * and clean up region fields.
	 */

	rlstlock();

	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	rp->r_flags = 0;
	rp->r_listsz = 0;
	rp->r_pgsz = 0;
	rp->r_nvalid = 0;
	rp->r_type = 0;
	rp->r_filesz = 0;
	rp->r_list = NULL;
	rp->r_noswapcnt = 0;

	/*
	 * Link into free list
	 */

	rp->r_forw = rfree.r_forw;
	rp->r_back = &rfree;
	rfree.r_forw->r_back = rp;
	rfree.r_forw = rp;

	rlstunlock();
}

/*
 * Attach a region to a process' address space
 */
preg_t *
attachreg(rp, up, vaddr, type, prot)
register reg_t *rp;	/* pointer to region to be attached */
struct user *up;	/* pointer to u-block (needed by fork) */
caddr_t	vaddr;		/* virtual address to attach at */
int	type;		/* Type to make the pregion. */
int	prot;		/* permissions for segment table entries. */
{
	register preg_t *prp1, *first;
	register preg_t *prp;
	preg_t  tmp;

	ASSERT(rp->r_flags & RG_LOCK);

	/*	Check attach address.
	 *	It must be segment aligned.
	 */

	 if ((int)vaddr & SOFFMASK) {
		u.u_error = EINVAL;
		return(NULL);
	 }

	/*	Allocate a pregion.  We should always find a
	 *	free pregion because of the way the system
	 *	is configured.
	 */

	prp = findpreg(up->u_procp, PT_UNUSED);
	if (prp == NULL) {
		u.u_error = EMFILE;
		return(NULL);
	}

	/*	init pregion
	 */

	prp1 = &tmp;
	prp1->p_reg = rp;
	prp1->p_regva = vaddr;
	prp1->p_type = type;
	prp1->p_flags = 0;
	if (prot == (SEG_RO))
		prp1->p_flags |= PF_RDONLY;

	/*	Check that region does not go beyond end of virtual
	 *	address space.
	 */

	 if (chkgrowth(up, prp1, 0, rp->r_pgsz)) {
		u.u_error = EINVAL;
		return(NULL);
	}

	/*	Load the segment table.
	 */

	if (rp->r_pgsz > 0)
		loadstbl(up, prp1, 0);

	/* Insert tmp in the pregion list
	*/

	first = (up->u_procp)->p_region;
	for (prp1=prp; prp1!=first; --prp1)
		if ((prp1-1)->p_regva > vaddr)
			*prp1 = *(prp1-1);
		else break;
	*prp1 = tmp;

	++rp->r_refcnt;
	up->u_procp->p_size += rp->r_pgsz;

	return(prp1);
}

/*	Detach a region from a process' address space.
**/

void
detachreg(prp, up)
register preg_t *prp;
register user_t	*up;
{
	register reg_t	*rp;
	register int	i;
	inode_t		*ip;

	rp = prp->p_reg;

	ASSERT(rp);
	ASSERT(rp->r_flags & RG_LOCK);

	/*
	 * Invalidate segment table entries pointing at the region
	 */
	
	if (rp->r_pgsz > 0)
		loadstbl(up, prp, -rp->r_pgsz);

	/*	Decrement process size by size of region.
	*/

	up->u_procp->p_size -= rp->r_pgsz;

	/*
	 * Decrement use count and free region if zero
	 * and RG_NOFREE is not set, otherwise unlock.
	 */
	if (--rp->r_refcnt == 0 && !(rp->r_flags & RG_NOFREE)) {
		freereg(rp);
	} else {
		if (ip = rp->r_iptr)
			prele(ip);
		regrele(rp);
	}
	
	/*	Clear the proc region we just detached.
	 */
	
	for (i = prp - up->u_procp->p_region; i < pregpp-1; i++, prp++) {
		*prp = *(prp+1);
	}
	*prp = nullpregion;
}

/*
 * Duplicate a region
 */

reg_t *
dupreg(rp, slpflg, force)
register reg_t *rp;
int slpflg, force;
{
	register int	i;
	register int	j;
	register int	size;
	register pde_t	*ppte;
	register pde_t	*cpte;
	register pde_t	**plp;
	register pde_t	**clp;
	register reg_t	*rp2;
	extern pde_t	*ptalloc();

	ASSERT(rp->r_flags & RG_LOCK);

	/* If region is shared, and we're not forcing a duplicate,
	 * there is no work to do.
	 * Just return the passed region.  The region reference
	 * counts are incremented by attachreg
	 */

	if (rp->r_type != RT_PRIVATE && force == 0)
		return(rp);
	
	/*	Make sure we have enough space to duplicate
	**	this region without potential deadlock.
	*/

	if (availsmem - rp->r_pgsz < tune.t_minasmem) {
		if (rp->r_iptr)
			prele(rp->r_iptr);
		nomemmsg("dupreg", rp->r_pgsz, 0, 0);
		u.u_error = EAGAIN;
		return(NULL);
	}
	availsmem -= rp->r_pgsz;

	/*
	 * Need to copy the region.
	 * Allocate a region descriptor
	 */

	if ((rp2 = allocreg(rp->r_iptr, force ? RT_PRIVATE : rp->r_type,
		0)) == NULL) {
		if (rp->r_iptr)
			prele(rp->r_iptr);
		availsmem += rp->r_pgsz;
		u.u_error = EAGAIN;
		return(NULL);
	}
	
	/*	Allocate a list for the new region.
	 */

	rp2->r_listsz = rp->r_listsz;
	rp2->r_list = (pde_t **)ptalloc(rp2->r_listsz, slpflg);
	rp2->r_flags = rp->r_flags;
	if (rp2->r_list == NULL) {
		rp2->r_listsz = 0;
		freereg(rp2);
		availsmem += rp->r_pgsz;
		u.u_error = EAGAIN;
		return(NULL);
	}


	/*
	 * Copy pertinent data to new region
	 */

	rp2->r_pgsz = rp->r_pgsz;
	rp2->r_filesz = rp->r_filesz;
	rp2->r_nvalid = rp->r_nvalid;

	/* Scan the parents page table  list and fix up each page table.
	 * Allocate a page table and map table for the child and
	 * copy it from the parent.
	 */

	for (i = 0  ;  i < ctos(rp->r_pgsz)  ;  i++) {
		plp = &rp->r_list[i];
		clp = &rp2->r_list[i];

		/* Allocate a page descriptor (table) and map table for the child.
		 */

		if ((cpte = ptalloc(2,slpflg)) == 0) {
			rp2->r_pgsz = stoc(i);
			freereg(rp2);
			availsmem += rp->r_pgsz;
			u.u_error = EAGAIN;
			return(NULL);
		}

		/* Set pointer to the newly allocated page descriptor (table)
		 * and dbd into the child's list.  Then get a
		 * pointer to the parents page descriptor (table) and dbd.
		 */

		*clp = cpte;
		ppte = *plp;

		/* Get the total number of unmapped pages remaining.
		 * This is the total size of the region minus the
		 * number of segments for which we have allocated
		 * page tables already.
		 */

		size = rp->r_pgsz - stoc(i);

		/* If this size is greater than a segment, then
		 * we will only process a segment.
		 */

		if (size > NPGPT)
			size = NPGPT;

		/* Check each parents page and then copy it to
		 * the childs pte.  Also check the map table
		 * entries.
		 */

		for (j = 0  ;  j < size  ;  j++, ppte++, cpte++) {
			dbd_t	map;

			/* Set copy-on-write bit
		 	 */
			pg_setcw(ppte);

			/*	Copy parents page to child.
			 */

			*cpte = *ppte;

			/*	If the page is in core, then
			 *	increment the page use count.
			 */

			if (ppte->pgm.pg_v) {
				struct pfdat *pfd;

				pfd = pfntopfdat(ppte->pgm.pg_pfn);
				ASSERT(pfd->pf_use > 0); 
				memlock();
				pfd->pf_use++;
				memunlock();
			}

			/* Increment the swap use count for pages which
			 * are on swap.
			 */

			map = *(dbd_t *)(ppte + NPGPT);
			if (map.dbd_type == DBD_SWAP) {
				ASSERT(swpuse(&map) != 0);
				if (!swpinc(&map, "dupreg")) {

					/* The swap use count overflowed.
					 * Free the region and return
					 * an error.
					 */

					((dbd_t *)(cpte + NPGPT))->dbd_type =
						DBD_NONE;
					freereg(rp2);
					u.u_error = EAGAIN;
					availsmem += rp->r_pgsz;
					return(NULL);
				}
			}
			*(dbd_t *)(cpte + NPGPT) = map;
		}
	}

	return(rp2);
}

/*
 * Change the size of a region
 *  change == 0  -> no-op
 *  change  < 0  -> shrink
 *  change  > 0  -> expand
 * For expansion, you get (fill) real pages (change-fill) demand zero pages
 * For shrink, the caller must flush the ATB
 * Returns 0 on no-op, -1 on failure, and 1 on success.
 */

growreg(prp, change, type)
register preg_t *prp;
{
	register pde_t	*pt;
	register int	i;
	register reg_t	*rp;
	register int	size;
	register int	osize;
	pde_t		pte;

	rp = prp->p_reg;
	ASSERT(rp->r_flags & RG_LOCK);

	if (change == 0)
		return(0);
	
	osize = rp->r_pgsz;

	ASSERT(rp->r_noswapcnt >=0);

	if (change < 0) {

		/*	The region is being shrunk.  Compute the new
		 *	size and free up the unneeded space.
		 */

		availsmem -= change;	/* change < 0.	*/

		if (rp->r_noswapcnt)
			availrmem -= change;
		i = ctost(osize + change);

		for (;  i < ctos(osize)  ;  i++) {
			/*	Free up the allocated pages for
			 *	this segment.
			 */

			pt = rp->r_list[i];
			size = osize - stoc(i);
			if (size > NPGPT)
				size = NPGPT;
			if (osize + change > stoc(i)) {
				size -= osize + change - stoc(i);
				pt += osize + change - stoc(i);
			}
			memlock();
			pfree(rp, pt, pt + NPGPT, size);
			memunlock();
		}

		/*	Free up the page tables which we no
		 *	longer need.
		 */

		(void) ptexpand(rp, change);
	} else {
		/*	We are expanding the region.  Make sure that
		 *	the new size is legal and then allocate new
		 *	page tables if necessary.
		 */

		if (availsmem - change < tune.t_minasmem) {
			nomemmsg("growreg", change, 0, 0);
			u.u_error = EAGAIN;
			return(-1);
		}
		if (rp->r_noswapcnt  &&
		   availrmem - change < tune.t_minarmem) {
			nomemmsg("growreg", change, 0, 1);
			u.u_error = EAGAIN;
			return(-1);
		}
		if (chkgrowth(&u, prp, osize, osize + change) ||
		   ptexpand(rp, change)) {
			u.u_error = ENOMEM;
			return(-1);
		}
		availsmem -= change;
		if (rp->r_noswapcnt)
			availrmem -= change;

		/*	Initialize the new page tables and allocate
		 *	pages if required.
		 */

		pte.pgi.pg_pde = 0;
		pg_setprot(&pte, PTE_RW);

		i = ctost(osize);
		for (; i < ctos(osize + change) ; i++) {
			pt = rp->r_list[i];
			size = osize + change - stoc(i);
			if (size > NPGPT)
				size = NPGPT;
			if (osize > stoc(i)) {
				size -= osize - stoc(i);
				pt += osize - stoc(i);
			}

			while (--size >= 0) {
				*pt = pte;
				((dbd_t *)(pt + NPGPT))->dbd_type = type;
				pt++;
			}
		}
	}

	loadstbl(&u, prp, change);

	rp->r_pgsz += change;
	u.u_procp->p_size += change;
	return(1);
}

/*
 * Check that grow of a pregion is legal
 * returns 0 if legal, -1 if illegal.
 */

chkgrowth(up, prp, osize, nsize)
register user_t	*up;
register preg_t	*prp;
register int	osize;	/* Old size in pages. */
register int	nsize;	/* New size in pages. */
{
	register size;
	register preg_t *prp1;
	register caddr_t start, end, vaddr;

	if (osize!= 0) { /* prp is in pregion list */
		if (nsize <= osize)
			return(0);
		prp1= prp + 1;
		if (prp1->p_reg != NULL) {
			if (prp->p_regva + ctob(nsize)-1 >= prp1->p_regva)
				return(-1);
		}
		if (chkstbl(up->u_procp,prp,nsize))
			return(-1);
		return(0);
	}

	/* prp is a new region */

	prp1 = (up->u_procp)->p_region;
	start = prp->p_regva;
	size = ctob((prp->p_reg)->r_pgsz);
	end = (size>0)? start+size-1:start;

	for (;prp1->p_reg; ++prp1) {
		if (prp1==prp) continue;
		vaddr = prp1->p_regva;
		if (end<vaddr)
			break;
		if (start == vaddr || (start< vaddr && end>=vaddr))
			return(-1);
		if (start>vaddr) {
			size = ctob((prp1->p_reg)->r_pgsz);
			if (size != 0 && start<vaddr+size)
				return(-1);
		}
	}

	if (chkstbl(up->u_procp,prp,nsize))
		return(-1);
	return(0);
}


/*
 * Expand user page tables for a region 
 */
 
ptexpand(rp, change)
register reg_t *rp;
{
	register pde_t	**lp;
	register int	osize;
	register int	nsize;
	register pde_t	**lp2;
	register int	i;
	extern pde_t	*uptalloc();

	/* Calculate the new size in pages.
	 */

	osize = rp->r_pgsz;
	nsize = osize + change;

	/*	If we are shrinking the region, then free up
	 *	the page tables and map tables.  Use a smaller
	 *	list if possible.
	 */

	if (ctos(nsize) < ctos(osize)) {
		lp = &rp->r_list[ctos(nsize)];
		for (;  lp < &rp->r_list[ctos(osize)]  ;  lp++) {
			uptfree(*lp, 2);
			*lp = 0;
		}
		if (ctos(ctos(nsize)) < rp->r_listsz) {
			if (nsize > 0) {
				lp2 = (pde_t **)uptalloc(ctos(ctos(nsize)));
				if (lp2 == NULL)
					/* we are shrinking and can't get space
					 * for a smaller list; just keep the
					 * bigger one.  Return OK
					 */
					return(0);
				bcopy(rp->r_list, lp2, ctos(nsize)<<2);
			}else{
				lp2 = 0;
			}
			uptfree(rp->r_list, rp->r_listsz);
			rp->r_list = lp2;
			rp->r_listsz = ctos(ctos(nsize));
		}
	}
		
	/*	If the region shrunk or didn't grow by enough to
	 *	require any more page tables, then we are done.
	 */

	if (change <= 0) {
		return(0);
	}
	
	/*	If the region grew into the next segment,  then we 
	 *	must allocate one or more new page and map tables.
	 *	See if we have enough space in the list for the
	 *	new segments.  If not, allocate a new list and
	 *	copy over the old data.
	 */
	
	i = ctos(ctos(nsize));
	if (i > rp->r_listsz) {
		lp2 = (pde_t **)uptalloc(i);
		if (lp2 == NULL)
			return(-1);
		if (rp->r_list) {
			bcopy(rp->r_list, lp2, ctos(osize)<<2);
			uptfree(rp->r_list, rp->r_listsz);
		}
		rp->r_list = lp2;
		rp->r_listsz = i;
	}

	/*
	 * Allocate a new set of page tables and disk maps.
	 */

	lp = &rp->r_list[ctos(osize)];
	lp2 = &rp->r_list[ctos(nsize)];

	for (; lp < lp2 ; lp++) {
		*lp = (pde_t *)uptalloc(2);
		if (*lp == NULL)
			return(-1);
	}

	return(0);
}

loadreg(prp, vaddr, ip, off, count)
register preg_t		*prp;
caddr_t			vaddr;
register struct inode	*ip;
{
	register reg_t	*rp;
	register int	gap;

	/*	Grow the region to the proper size to load the file.
	 */

	rp = prp->p_reg;
	ASSERT(rp->r_flags & RG_LOCK);
	gap = vaddr - prp->p_regva;

	if (growreg(prp, btoct(gap), DBD_NONE) < 0) 
		return(-1);
	if (growreg(prp, btoc(count+gap) - btoct(gap), DBD_DFILL) < 0) 
		return(-1);

	flushsect(secnum(vaddr));

	/*	Set up to do the I/O.
	 */

	u.u_segflg = 0;
	u.u_base = vaddr;
	u.u_count = count;
	u.u_offset = off;

	/*	We must unlock the region here because we are going
	 *	to fault in the pages as we read them.  No one else
	 *	will try to use the region before we finish because
	 *	the RG_DONE flag is not set yet.
	 */

	regrele(rp);

	FS_READI(ip);

	if (u.u_error) {
		reglock(rp);
		return(-1);
	}

	/*	Clear the last (unused)  part of the last page.
	 */
	
	vaddr += count;
	count = ctob(1) - poff(vaddr);
	if (count > 0  &&  count < ctob(1))
		bzeroba(vaddr, count);

	reglock(rp);
	rp->r_flags |= RG_DONE;
	if (rp->r_flags & RG_WAITING) {
		rp->r_flags &= ~RG_WAITING;
		wakeup(&rp->r_flags);
	}
	if (u.u_count)
		return(-1);
	return(0);
}

mapreg(prp, vaddr, ip, off, count)
preg_t	*prp;
caddr_t		vaddr;
struct inode	*ip;
int		off;
register int	count;
{
	register int	i;
	register int	j;
	register int	blkspp;
	register reg_t	*rp;
	int		gap;
	int		seglim;
	dbd_t		*dbd;

	/*	Make sure that we are not trying to map
	 *	beyond the end of the file.  This can
	 *	happen for a bad a.out where the header
	 *	lies and says the file is bigger than
	 *	it actually is.
	 */

	if (off + count > ip->i_size)
		return(-1);

	/*
	 * Make sure the map is built.
	 */

	if (FS_ALLOCMAP(ip) == 0)
		return(-1);

	/*	Get region pointer and effective device number.
	 */

	rp = prp->p_reg;
	ASSERT(rp->r_flags & RG_LOCK);

	/*	Compute the number of file system blocks in a page.
	 *	This depends on the file system block size.
	 */

	blkspp = NBPP/FSBSIZE(ip);

	/*	Allocate invalid pages for the gap at the start of
	 *	the region and demand-fill pages for the actual
	 *	text.
	 */
	
	gap = vaddr - prp->p_regva;
	if (growreg(prp, btoct(gap), DBD_NONE) < 0)
		return(-1);
	if (growreg(prp, btoc(count+gap) - btoct(gap), DBD_DFILL) < 0)
		return(-1);
	
	rp->r_filesz = count + off;
	
	/*	Build block list pointing to map table.
	 */

	gap = btoct(gap);  /* Gap in pages. */
	off = btoct(off) * blkspp;  /* File offset in blocks. */
	i = ctost(gap);
	seglim = ctos(rp->r_pgsz);

	for (;  i < seglim  ;  i++) {
		register int	lim;
		register pde_t	*pt;

		if (gap > stoc(i))
			j = gap - stoc(i);
		else
			j = 0;

		lim = rp->r_pgsz - stoc(i);
		if (lim > NPGPT)
			lim = NPGPT;

		pt = (pde_t *)rp->r_list[i] + j;
		dbd = (dbd_t *)pt + NPGPT;

		for (;  j < lim  ;  j++, pt++, dbd++) {

			/*	If these are private pages, then make
			 *	them copy-on-write since they will
			 *	be put in the hash table.
			 *	If it is shared text, also make it
			 *	copy on write. This is to work around
			 *	the mau hardware problem. If the mau
			 *	performs an operation on shared text,
			 *	it can alter it. Setting the copy on
			 *	write bit causes a protection fault to
			 *	occur when this happens. The pfault code
			 *	then check if it is shared text, and then
			 *	returns an error.
			 */

			if (rp->r_type == RT_PRIVATE || rp->r_type == RT_STEXT) {
				pg_setcw(pt);
			}
			dbd->dbd_type  = DBD_FILE;
			dbd->dbd_blkno = off;
			off += blkspp;
		}
	}

	/*	Mark the last page for special handling
	 */
	
	dbd[-1].dbd_type = DBD_LSTFILE;

	rp->r_flags |= RG_DONE;
	if (rp->r_flags & RG_WAITING)
	{	rp->r_flags &= ~RG_WAITING;
		wakeup(&rp->r_flags);
	}
	return(0);

}

/*	Find the region corresponding to a virtual address.
 */

reg_t	*
findreg(p, vaddr)
register struct proc	*p;
register caddr_t	vaddr;
{
	register preg_t	*prp;
	register preg_t	*oprp;

	oprp = p->p_region;
	for (prp = &p->p_region[1] ; prp->p_reg ; prp++)
		if (vaddr >= prp->p_regva  &&
		   prp->p_regva > oprp->p_regva)
			oprp = prp;
	if (oprp->p_reg  &&  vaddr >= oprp->p_regva  &&
	   vaddr < (oprp->p_regva + ctob(oprp->p_reg->r_pgsz))) {
		reglock(oprp->p_reg);
		return(oprp->p_reg);
	}
	return(NULL);
}

/*	Find the pregion of a particular type.
 */

preg_t *
findpreg(pp, type)
register proc_t	*pp;
register int	type;
{
	register preg_t	*prp;

	for (prp = pp->p_region ; prp->p_reg ; prp++) {
		if (prp->p_type == type)
			return(prp);
	}

	/*	We stopped on an unused region.  If this is what
	 *	was called for, then return it unless it is the
	 *	last region for the process.  We leave the last
	 *	region unused as a marker.
	 */

	if ((type == PT_UNUSED)  &&  (prp < &pp->p_region[pregpp - 1]))
		return(prp);
	return(NULL);
}

/*
 * Change protection of pdes for a region
 */
void
chgprot(prp, prot)
preg_t	*prp;
{
	if (prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;
	else
		prp->p_flags &= ~PF_RDONLY;

	loadstbl(&u, prp, 0);

	flushsect(secnum(prp->p_regva));
}

/* Locate process region for a given virtual address. */

preg_t *
vtopreg(p, vaddr)
register struct proc *p;
register caddr_t vaddr;
{
	register preg_t *prp;
	register reg_t *rp;

	for (prp = p->p_region; rp = prp->p_reg; prp++) {
#ifdef vax
		caddr_t lo = prp->p_regva + ctob(rp->r_pgoff);
#else
		caddr_t lo = prp->p_regva;
#endif
		caddr_t hi = lo + ctob(rp->r_pgsz);
		if ((unsigned long)vaddr >= (unsigned long)lo
		  && (unsigned long)vaddr < (unsigned long)hi)
			return(prp);
	}
	return(NULL);
}
