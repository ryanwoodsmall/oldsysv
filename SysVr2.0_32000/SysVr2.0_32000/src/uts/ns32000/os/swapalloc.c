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
/* @(#)swapalloc.c	1.2 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/pfdat.h"
#include "sys/page.h"
#include "sys/seg.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/systm.h"

#include "sys/debug.h"


extern int	swapwant;	/* Set if somebody's looking for swap */



/*	Allocate swap file space.
 */

swalloc(dbd, size)
register dbd_t	*dbd;		/* Ptr to dbd's to set up.	      */
register int	size;		/* Number of pages of swap needed.    */
{
	register int	smi;
	register use_t	*cntptr;
	register int	i;
	register int	swappg;

	swappg = 0;

	/*	Search all of the swap files, starting with the one
	 *	following the one which we allocated on last, looking
	 *	for a file with enough space to satisfy the current
	 *	request.
	 */
	for(;;) {
		smi = nextswap;

		/*	There can be holes in the swap file table
		 *	(swaptab) due to deletions.
		 */

		do{
			/*	If the current swaptab entry is not
			 *	in use or is in the process of being
			 *	deleted, go on to the next one.
			 */

			if((swaptab[smi].st_ucnt == NULL)  ||
			   (swaptab[smi].st_flags & ST_INDEL))
				continue;
			if(swappg = swapfind(&swaptab[smi], size))
				break;
		} while((smi = (smi + 1) % MSFILES) != nextswap);

		/*	If we got the swap space, then go set up the
		 *	disk block descriptors.
		 */

		if(swappg)
			break;

		/*	Try to free up some swap space by removing
		 *	unused sticky text regions.  If this
		 *	suceeds, try to allocate again.  Otherwise
		 *	go to sleep waiting for swap space.
		 */

		if(swapclup()){
			printf("\nWARNING: Swap space running out.\n");
			printf("  Needed %d pages\n", size);
			continue;
		}

		printf("\nDANGER: Out of swap space.\n");
		printf("  Needed %d pages\n", size);

		swapwant++;
		sleep((caddr_t) &swapwant);
	}

	/*	Page numbers in the map begin at 1 since swapfind
	 *	returns 0 for an error.  Fix this now.  Then
	 *	set up for main dbd processing loop.
	 */

	swappg -= 1;
	cntptr = &swaptab[smi].st_ucnt[swappg];
	swappg = swaptab[smi].st_swplo + (swappg << DPPSHFT);
	swaptab[smi].st_nfpgs -= size;
	nextswap = (smi + 1) % MSFILES;

	/*	Initialize the swap use counts for each page
	 *	and set up the disk block descriptors (dbd's).
	 */

	for(i = 0  ;  i < size  ;  i++, cntptr++, dbd++){
		*cntptr = 1;
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
	register retval;

	st = &swaptab[dbd->dbd_swpi];
	pgnbr = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;
	cntptr = &st->st_ucnt[pgnbr];
	pgnbr += 1;

	ASSERT(*cntptr != 0);

	/*	Decrement the use count for this page.  If it goes
	 *	to zero, then free the page.  If anyone is waiting
	 *	for swap space, wake them up.
	 */

	retval = (*cntptr -= 1);

	if(retval == 0) {
		st->st_nfpgs += 1;
		if(swapwant) {
			swapwant = 0;
			wakeup((caddr_t) &swapwant);
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

	if(st->st_ucnt[pg] >= MAXSUSE){
		printf("%s - swpuse count overflow\n", nm);
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
	for(i = 0  ;  i < MSFILES  ;  i++){
		st = &swaptab[i];
		if(st->st_ucnt == NULL){
			if(smi == -1)
				smi = i;
		} else if(st->st_dev == dev  && st->st_swplo == lowblk){
			if((st->st_flags & ST_INDEL)  &&
			   (st->st_npgs == (nblks >> DPPSHFT))){
				st->st_flags &= ~ST_INDEL;
				return(smi);
			}
			u.u_error = EEXIST;
			return(-1);
		}
	}

	/*	If no free entry is available, give an error
	 *	return.
	 */

	if(smi < 0){
		u.u_error = ENOSPC;
		return(-1);
	}
	st = &swaptab[smi];

	/*	Open the swap file.
	 */

	u.u_error = 0;
	(*bdevsw[bmajor(dev)].d_open)(minor(dev), 1);
	if(u.u_error)
		return(-1);

	/*	Initialize the new entry.
	 */

	st->st_dev = dev;
	st->st_swplo = lowblk;
	st->st_npgs = nblks >> DPPSHFT;
	st->st_nfpgs = st->st_npgs;

	/*	Allocate space for the use count array.  One counter
	 *	for each page of swap.
	 */

	i = btop(st->st_npgs * sizeof(use_t));
	st->st_ucnt = (use_t *)sptalloc(i, PG_V | PG_KW, 0);
	st->st_last = st->st_ucnt;

	/*	Clearing the flags allows swalloc to find it
	 *	Wakeup if somebody is waiting for swap space
	 */
	st->st_flags = 0;
	if(swapwant) {
		swapwant = 0;
		wakeup((caddr_t) &swapwant);
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
	for(i = 0  ;  i < MSFILES  ;  i++){
		st = &swaptab[i];
		if(st->st_ucnt == NULL)
			continue;
		if(st->st_dev == dev  && st->st_swplo == lowblk)
			smi = i;
		else if((st->st_flags & ST_INDEL) == 0)
			ok++;
	};
	
	/*	If the file was not found, then give an error
	 *	return.
	 */

	if(smi < 0){
		u.u_error = EINVAL;
		return(-1);
	}

	/*	If we are trying to delete the last swap file,
	 *	then give an error return.
	 */
	
	if(!ok){
		u.u_error = ENOMEM;
		return(-1);
	}
	
	st = &swaptab[smi];

	/*	Set the delete flag.  Clean up its pages.
	 *	The file will be removed by swfree1 when
	 *	all of the pages are freed.
	 */

	st->st_flags |= ST_INDEL;
	if(st->st_nfpgs < st->st_npgs)
		getswap(smi);

	if(st->st_nfpgs == st->st_npgs)
		swaprem(st);

	return(smi);
}


/*	Remove a swap file from swaptab.
 */

swaprem(st)
swpt_t	*st;
{
	register int	i;


	/*	Release the space used by the use count array.
	 */

	i = btop(st->st_npgs * sizeof(use_t));
	sptfree(st->st_ucnt, i, 1);

	/*	Mark the swaptab entry as unused.
	 */

	st->st_ucnt = NULL;
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
	register pte_t	*pt;
	register dbd_t	*dbd;
	register int	pglim;
	register int	j;
	register int	i;
	register int	seglim;

	for (rp = region; rp < &region[v.v_region]; rp++) {

		/*	Skip the region, if its unused
		 */
		if (rp->r_type == RT_UNUSED)
			continue;

		reglock(rp);


		/*	Loop through all the segments of the region.
		*/

		i = ptots(rp->r_pgoff);
		seglim = ptos(rp->r_pgoff + rp->r_pgsz);

		for(  ;  i < seglim  ;  i++){

			/*	Look at all of the pages of the segment.
			 */

			if(rp->r_pgoff > stopg(i))
				j = rp->r_pgoff - stopg(i);
			else
				j = 0;
			pglim = rp->r_pgoff + rp->r_pgsz - stopg(i);
			if(pglim > NPGPT)
				pglim = NPGPT;

			pt = rp->r_list[i] + j;
			dbd = (dbd_t *) pt + NPGPT;
			
			for(  ;  j < pglim  ;  j++, pt++, dbd++){
				if(dbd->dbd_type == DBD_SWAP  &&
				   dbd->dbd_swpi ==  smi){
					unswap(rp, pt, dbd);
				}
			}
		}
		regrele(rp);
	}

}

/*	Free up the swap block being used by the indicated page.
 *	The region is locked when we are called.
 *	This code roughly parallels vfault().
 */

unswap(rp, pt, dbd)
register reg_t	*rp;
register pte_t	*pt;
register dbd_t	*dbd;
{
	register pfd_t		*pfd;
	register pte_t		*kernpte;
	register caddr_t	*kernvaddr;


	/*	If a copy of the page is in core, then just
	 *	release the copy on swap.
	 */

	if (pt->pgi.pg_pte & (PG_V | PG_REF | PG_NDREF)) {
		pfd = &pfdat[pt->pgm.pg_pfn];
		if (pfd->pf_flags & P_HASH) {
			premove(pfd);
		}
		swfree1(dbd);
		dbd->dbd_type = DBD_NONE;
		return;
	}

	/*	Allocate a kernel window for the swap and a memory page
	 */

	if ((kernvaddr = (caddr_t *)sptalloc(1, PG_V|PG_KW, -1)) == NULL)
		return;

	if (ptmemall(rp, pt, 1, 0)) {
		sptfree(kernvaddr, 1, 0);
		return;
	}

	kernpte = (pte_t *)svtopte(kernvaddr);
	kernpte->pgm.pg_pfn = pt->pgm.pg_pfn;
	kernpte->pgm.pg_pfn2 = pt->pgm.pg_pfn2;

	/*	Read in the page from swap and then free up the swap.
	 */

	swap(dbd, kernpte, kernvaddr, 1, B_READ, 0);
	swfree1(dbd);
	dbd->dbd_type = DBD_NONE;
	pfd = &pfdat[pt->pgm.pg_pfn];
	pfd->pf_flags |= P_DONE;
	pt->pgi.pg_pte |= PG_V | PG_REF | PG_NDREF;
	pt->pgi.pg_pte &= ~PG_M;
	pt->pgi.pg_pte2 = PG_PE2(pt->pgi.pg_pte);

	/*	Free the kernel window
	 */
	sptfree(kernvaddr, 1, 0);
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
	for(p = st->st_last; p <= e; p++) {
		if(*p == 0) {
			b = p;
			p++;
			for(i = 1; i < size; i++, p++)
				if(*p != 0) goto Cont;
			st->st_last = p;
			return((b - st->st_ucnt) + 1);
		}
	  Cont:;
	}
	e = st->st_last - size;
	for(p = st->st_ucnt; p <= e; p++) {
		if(*p == 0) {
			b = p;
			p++;
			for(i = 1; i < size; i++, p++)
				if(*p != 0) goto Cont2;
			st->st_last = p;
			return((b - st->st_ucnt) + 1);
		}
	  Cont2:;
	}
	return(0);
}
