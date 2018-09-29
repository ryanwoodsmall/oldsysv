/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/swapalloc.c	10.8"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/systm.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/map.h"
#include "sys/open.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"
#include "sys/file.h"

proc_t		*swapwold;	/* Head of "waiting for swap" list. */
proc_t		*swapwnew;	/* Tail of "waiting for swap" list. */


/*	Allocate swap file space.
 */

swalloc(pglist, size, waitflag)
register pglst_t	*pglist;/* Ptr to a list of pointers to page  */
				/* table entries for which swap is to */
				/* be allocated.		      */
register int	size;		/* Number of pages of swap needed.    */
register int	waitflag;	/* If required space is not available */
				/* then wait for it if this flag is   */
				/* set and return an error otherwise. */
{
	register int	smi;
	register use_t	*cntptr;
	register int	i;
	register int	swappg;
	register dbd_t	*dbd;
	register proc_t	*pp;

	swappg = 0;

	/*	Search all of the swap files, starting with the one
	 *	following the one which we allocated on last, looking
	 *	for a file with enough space to satisfy the current
	 *	request.
	 */
	for (;;) {

		smi = nextswap;

		/*	There can be holes in the swap file table
		 *	(swaptab) due to deletions.
		 */

		do{
			/*	If the current swaptab entry is not
			 *	in use or is in the process of being
			 *	deleted, go on to the next one.
			 */

			if ((swaptab[smi].st_ucnt == NULL)  ||
			   (swaptab[smi].st_flags & ST_INDEL))
				continue;
			swappg = swapfind(&swaptab[smi], size);
			if (swappg >= 0)
				break;
		} while ((smi = (smi + 1) % MSFILES) != nextswap);

		/*	If we got the swap space, then go set up the
		 *	disk block descriptors.
		 */

		if (swappg >= 0)
			break;

		/*	Try to free up some swap space by removing
		 *	unused sticky text regions.  If this
		 *	suceeds, try to allocate again.  Otherwise,
		 *	either return an error or go to sleep
		 *	waiting for swap space depending on the
		 *	setting of the "waitflag" argument.
		 */

		if (swapclup()) {
			continue;
		}

		if (waitflag == 0) {
			return(-1);
		}

		cmn_err(CE_CONT, "DANGER: Out of swap space.\n");
		cmn_err(CE_CONT, "\tWaiting for %d pages.\n", size);

		pp = u.u_procp;
		pp->p_mpgneed = size;
		pp->p_mlink = NULL;
		if (swapwold) {
			swapwnew->p_mlink = pp;
			swapwnew = pp;
		} else {
			swapwold = swapwnew = pp;
		}
		sleep(&swapwold, PMEM);
	}


	/*	Set up for main processing loop.
	*/

	cntptr = &swaptab[smi].st_ucnt[swappg];
	swappg = swaptab[smi].st_swplo + (swappg << DPPSHFT);
	swaptab[smi].st_nfpgs -= size;
	nextswap = (smi + 1) % MSFILES;


	/*	Initialize the swap use counts for each page
	 *	and set up the disk block descriptors (dbd's).
	 */

	for (i = 0  ;  i < size  ;  i++, cntptr++, pglist++) {
		*cntptr = 1;
		dbd = (dbd_t *)(pglist->gp_ptptr + NPGPT);
		dbd->dbd_type = DBD_SWAP;
		dbd->dbd_swpi = smi;
		dbd->dbd_blkno = swappg + (i << DPPSHFT);
	}
	
	return(swappg);
}


/*	Free one page of swap and return the resulting use count.
 */

swfree1(dbd)
register dbd_t	*dbd;	/* Ptr to disk block descriptor for	*/
			/* block to be removed.			*/
{
	register use_t	*cntptr;
	register int	pgnbr;
	register swpt_t	*st;
	register int	retval;


	st = &swaptab[dbd->dbd_swpi];
	pgnbr = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;
	cntptr = &st->st_ucnt[pgnbr];

	ASSERT(*cntptr != 0);

	/*	Decrement the use count for this page.  If it goes
	 *	to zero, then free the page.  If anyone is waiting
	 *	for swap space, wake them up.
	 */

	retval = (*cntptr -= 1);

	if (retval == 0) {
		st->st_nfpgs += 1;

		/*	Wake up the first process waiting for swap
		 *	if we have freed up enough space.  Since we
		 *	are only freeing one page, we cannot
		 *	satisfy more than one process's request.
		 */

		if (swapwold  &&  swapwold->p_mpgneed <= st->st_nfpgs) {
			setrun(swapwold);
			swapwold = swapwold->p_mlink;
		}
	}

	return(retval);
}


/*	Find the use count for a block of swap.
 */

swpuse(dbd)
register dbd_t	*dbd;
{
	register swpt_t	*st;
	register int	pg;

	st = &swaptab[dbd->dbd_swpi];
	pg = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;

	return(st->st_ucnt[pg]);
}


/*	Increment the use count for a block of swap.
 */

swpinc(dbd, nm)
register dbd_t	*dbd;
char		*nm;
{
	register swpt_t	*st;
	register int	pg;

	st = &swaptab[dbd->dbd_swpi];
	pg = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;

	if (st->st_ucnt[pg] >= MAXSUSE) {
		cmn_err(CE_NOTE, "%s - swpuse count overflow.\n", nm);
		return(0);
	}
	st->st_ucnt[pg]++;
	return(1);
}


/*	Add a new swap file.
 */

swapadd(dev, lowblk, nblks)
dev_t		dev;		/* The device code.		*/
int		lowblk;		/* First block on device to use.*/
int		nblks;		/* Nbr of blocks to use.	*/
{
	register int	smi;
	register swpt_t	*st;
	register int	i;

	/*	Find a free entry in the swap file table.
	 *	Check to see if the new entry duplicates an
	 *	existing entry.  If so, this is an error unless
	 *	the existing entry is being deleted.  In this
	 *	case, just clear the INDEL flag and the swap
	 *	file will be used again.
	 */


	smi = -1;
	for (i = 0  ;  i < MSFILES  ;  i++) {
		st = &swaptab[i];
		if (st->st_ucnt == NULL) {
			if (smi == -1)
				smi = i;
		} else if (st->st_dev == dev  && st->st_swplo == lowblk) {
			if ((st->st_flags & ST_INDEL)  &&
			   (st->st_npgs == (nblks >> DPPSHFT))) {
				st->st_flags &= ~ST_INDEL;
				availsmem += st->st_npgs;
				return(smi);
			}
			u.u_error = EEXIST;
			return(-1);
		}
	}

	/*	If no free entry is available, give an error
	 *	return.
	 */

	if (smi < 0) {
		u.u_error = ENOSPC;
		return(-1);
	}
	st = &swaptab[smi];

	/*	Open the swap file.
	 */

	u.u_error = 0;
	(*bdevsw[major(dev)].d_open)(dev, FREAD|FWRITE, OTYP_SWP);
	if (u.u_error) {
		st->st_ucnt = NULL;
		st->st_flags = 0;
		return(-1);
	}

	/*	Initialize the new entry.
	 */

	st->st_dev = dev;
	st->st_swplo = lowblk;
	st->st_npgs = nblks >> DPPSHFT;
	st->st_nfpgs = st->st_npgs;


	/*	Allocate space for the use count array.  One counter
	 *	for each page of swap.
	 */

	i = st->st_npgs * sizeof(use_t);  /* Nbr of bytes for use   */
					  /* count.		    */
	i = (i + PTSIZE - 1) / PTSIZE;	  /* Nbr of 256 byte page   */
					  /* tables.		    */
	st->st_ucnt = (use_t *)uptalloc(i);
	if (st->st_ucnt == NULL)
		return(-1);
	st->st_next = st->st_ucnt;
	availsmem += st->st_npgs;

	/*	Clearing the flags allows swalloc to find it
	 */
	st->st_flags = 0;
	while (swapwold) {
		setrun(swapwold);
		swapwold = swapwold->p_mlink;
	}
	return(smi);
}


/*	Delete a swap file.
 */

swapdel(dev, lowblk)
register dev_t	dev;	/* Device to delete.			*/
register int	lowblk;	/* Low block number of area to delete.	*/
{
	register swpt_t	*st;
	register int	smi;
	register int	i;
	register int	ok;

	/*	Find the swap file table entry for the file to
	 *	be deleted.  Also, make sure that we don't
	 *	delete the last swap file.
	 */

	ok = 0;
	smi = -1;
	for (i = 0  ;  i < MSFILES  ;  i++) {
		st = &swaptab[i];
		if (st->st_ucnt == NULL)
			continue;
		if (st->st_dev == dev  && st->st_swplo == lowblk)
			smi = i;
		else if ((st->st_flags & ST_INDEL) == 0)
			ok++;
	};
	
	/*	If the file was not found, then give an error
	 *	return.
	 */

	if (smi < 0) {
		u.u_error = EINVAL;
		return(-1);
	}

	/*	If we are trying to delete the last swap file,
	 *	then give an error return.
	 */
	
	if (!ok) {
		u.u_error = ENOMEM;
		return(-1);
	}
	
	st = &swaptab[smi];

	/*	Set the delete flag.  Clean up its pages.
	 *	The file will be removed by swfree1 when
	 *	all of the pages are freed.
	 */

	if (!(st->st_flags & ST_INDEL)) {
		if (availsmem - st->st_npgs < tune.t_minasmem) {
			cmn_err(CE_NOTE,
				"swapdel - too few free pages");
			u.u_error = ENOMEM;
			return(-1);
		}
		availsmem -= st->st_npgs;
		st->st_flags |= ST_INDEL;
	}
	if (st->st_nfpgs < st->st_npgs) {
		getswap(smi);
	}

	if (st->st_nfpgs == st->st_npgs)
		swaprem(st);

	return(smi);
}


/*	Remove a swap file from swaptab.
 *	Called with swaplock set.
 */

swaprem(st)
swpt_t	*st;
{
	register int	i;
	register dev_t	dev;

	/*	Release the space used by the use count array.
	 */

	dev = st->st_dev;
	i = st->st_npgs * sizeof(use_t);  /* Nbr of bytes for use   */
					  /* count.		    */
	i = (i + PTSIZE - 1) / PTSIZE;	  /* Nbr of 256 byte page   */
					  /* tables.		    */
	uptfree(st->st_ucnt, i);

	/*	Mark the swaptab entry as unused.
	 */

	st->st_ucnt = NULL;
	(*bdevsw[major(dev)].d_close)(dev, 1, OTYP_SWP);
}

/*	Try to free up swap space on the swap device being deleted.
 *	Look at every region for pages which are swapped to the
 *	device we want to delete.  Read in these pages and delete
 *	the swap.
 */

getswap(smi)
int	smi;
{
	register reg_t	*rp;
	reg_t		*nrp;
	register pde_t	*pt;
	register dbd_t	*dbd;
	register pde_t	*pglim;
	register int	i;
	register int	seglim;

	rlstlock();

	for (rp = ractive.r_forw; rp != &ractive; rp = nrp) {

		/*	If we can't lock the region, then
		 *	skip it for now.
		 */

		if (rp->r_flags & RG_LOCK) {
			nrp = rp->r_forw;
			continue;
		}
		reglock(rp);

		/*	Loop through all the segments of the region.
		*/

		seglim = ctos(rp->r_pgsz);

		for (i = 0  ;  i < seglim  ;  i++) {

			/*	Look at all of the pages of the segment.
			 */

			pt = rp->r_list[i];
			dbd = (dbd_t *)pt + NPGPT;
			if (rp->r_pgsz - stoc(i) < NPGPT)
				pglim = pt + (rp->r_pgsz - stoc(i));
			else
				pglim = pt + NPGPT;
			
			for (;  pt < pglim  ;  pt++, dbd++) {
				if (dbd->dbd_type == DBD_SWAP  &&
				   dbd->dbd_swpi ==  smi) {
					rlstunlock();
					unswap(rp, pt, dbd);
					rlstlock();
				}
			}
		}
		nrp = rp->r_forw;
		regrele(rp);
	}

	rlstunlock();
}

/*	Free up the swap block being used by the indicated page.
 *	The region is locked when we are called.
 */

unswap(rp, pt, dbd)
register reg_t	*rp;
register pde_t	*pt;
register dbd_t	*dbd;
{
	register pfd_t	*pfd;
	pglst_t		pglist;

	ASSERT(rp->r_flags & RG_LOCK);

	/*	If a copy of the page is in core, then just
	 *	release the copy on swap.
	 */

	if (pt->pgm.pg_v) {
		pfd = pfntopfdat(pt->pgm.pg_pfn);
		memlock();
		if (pfd->pf_flags & P_HASH) {
			premove(pfd);
		}
		swfree1(dbd);
		dbd->dbd_type = DBD_NONE;
		memunlock();
		return;
	}

	/*	Allocate a page of physical memory for the page.
	 */

	memlock();
	if (ptmemall(rp, pt, 1, 0, 0)) {
		memunlock();
		return;
	}
	memunlock();

	/*	Read in the page from swap and then free up the swap.
	 */

	pglist.gp_ptptr = pt;
	swap(&pglist, 1, B_READ);
	swfree1(dbd);
	dbd->dbd_type = DBD_NONE;
	pfd = pfntopfdat(pt->pgm.pg_pfn);
	pfd->pf_flags |= P_DONE;
	pg_setvalid(pt);
	pg_setref(pt);
	pg_setndref(pt);
	pg_clrmod(pt);
}

/*	Search swap use counters looking for size contiguous free pages.
 *	Returns the page number found + 1 on sucess, 0 on failure.
 */

swapfind(st, size)
register swpt_t	*st;
register int size;
{
	register use_t *p, *e;
	register int i;
	use_t *b;

	e = &st->st_ucnt[st->st_npgs - size];
	for (p = st->st_next; p <= e; p++) {
		if (*p == 0) {
			b = p;
			p++;
			for (i = 1; i < size; i++, p++)
				if (*p != 0) goto Cont;
			st->st_next = p;
			return(b - st->st_ucnt);
		}
	  Cont:;
	}
	e = st->st_next - size;
	for (p = st->st_ucnt; p <= e; p++) {
		if (*p == 0) {
			b = p;
			p++;
			for (i = 1; i < size; i++, p++)
				if (*p != 0) goto Cont2;
			st->st_next = p;
			return(b - st->st_ucnt);
		}
	  Cont2:;
	}

	st->st_next = st->st_ucnt;
	return(-1);
}

