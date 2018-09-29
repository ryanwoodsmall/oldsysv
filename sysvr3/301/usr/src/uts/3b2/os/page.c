/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/page.c	10.8"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/var.h"
#include "sys/mount.h"
#include "sys/buf.h"
#include "sys/map.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

int			mem_lock;
int			firstfree, maxfree, freemem;
int			availrmem;
int			availsmem;
extern unsigned int	sxbrkcnt;


/*	Lock memory allocation.
 */


memlock()
{
	while (mem_lock)
		sleep(&mem_lock, PMEM);
	mem_lock = 1;
}




/*	Unlock memory allocation.
 */


memunlock()
{
	ASSERT(mem_lock);
	mem_lock = 0;
	wakeup(&mem_lock);
}

memlocked()
{
	return(mem_lock);
}


/*
 *	getcpages(npgs,nosleep) - gets physically continguous pages, 
 *		the kernel virtual address of the pages allocated.
 *		is returned.
 *	npgs = number of pages to be allocated.
 *	wait for pages only if nosleep =0.
 *
 *	Remark: pages allocated are not mapped to sysreg
 */

pde_t *
getcpages(npgs, nosleep)
{
	register int	pfn;

	do {
		memlock();
		pfn = contmemall(npgs);
		memunlock();
		if (pfn != NULL)
			return((pde_t *)phystokv(ctob(pfn)));
		if (npgs > 1  &&  nosleep) {
			nomemmsg("getcpages", npgs, 1, 0);
			return(NULL);
		}
		cmn_err(CE_NOTE,
			"!getcpages - waiting for %d contiguous pages",
			npgs);
		u.u_procp->p_stat = SXBRK;
		++sxbrkcnt;
		if (runout) {
			runout = 0;
			wakeup(&runout);
		}
		swtch();
	} while (1);
}

/*
 * freepage(pfn)
 */

freepage(pfn)
{
	pde_t base;

	base.pgi.pg_pde = mkpde(PG_P, pfn);
	memlock();
	pfree(NULL,&base,NULL,1);
	memunlock();
}

/*
 * contmemall(npgs)
 *
 * allocate phycally contiguous pages
 */

contmemall(npgs)
{
	register struct pfdat	*pfd, *pfd1,*top;
	register int numpages;

	/*
	 * Need contiguous memory
	 */
	
	ASSERT(memlocked());

	if (freemem < npgs)
		return(NULL);

	pfd = pfntopfdat(btoc(kpbase));
	top = pfntopfdat(maxclick);

	for (; pfd < top; pfd++) {
		if (pfd->pf_flags & P_QUEUE) {
			for (pfd1 = pfd + 1, numpages = npgs ;
			    pfd1 <= top && --numpages > 0 ;
			    pfd1++)
				if (!(pfd1->pf_flags & P_QUEUE))
					break;
			if (numpages == 0)
				break;
			pfd = pfd1;
		}
	}

	if (pfd >= top)
		return(NULL);

	ASSERT(pfd1 <= top);
	ASSERT(pfd1 - pfd == npgs);

	/*
	 * Take pages *pfd .. *(--pfd1)
	 */

	for (; --pfd1 >= pfd;) {
		(pfd1->pf_prev)->pf_next = pfd1->pf_next;
		(pfd1->pf_next)->pf_prev = pfd1->pf_prev;
		pfd1->pf_next = NULL;
		pfd1->pf_prev = NULL;
		if (pfd1->pf_flags&P_HASH)
			premove(pfd1);
		pfd1->pf_blkno = BLKNULL;
		pfd1->pf_use = 1;
		pfd1->pf_flags = 0;
		pfd1->pf_rawcnt = 0;
	}
	freemem -= npgs;
	return(pfdattopfn(pfd));
}

/*
 * Allocate pages and fill in page table
 *	rp		-> region pages are being added to.
 *	base		-> address of page table
 *	size		-> # of pages needed
 *	validate	-> Mark pages valid if set.
 *	nosleep		-> wait for pages only if nosleep is 0.
 * returns:
 *	0	Memory allocated immediately.
 *	1	Had to unlock region and go to sleep before
 *		memory was obtained.  After awakening, the
 *		page was valid or pfree'd so no page was
 *		allocated.
 *	-1	Not enough pages to cover the request.
 *
 * Called with mem_lock set and returns the same way.
 */

ptmemall(rp, base, size, validate, nosleep)
reg_t		*rp;
register pde_t	*base;
register int	validate;
{
	register struct pfdat	*pfd;
	register int		i;

	/*	Check for illegal size.
	 */

	ASSERT(size > 0);
	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(memlocked());

	switch (memreserve(rp, size, nosleep)) {
		case  0: break;
		case -1: return(-1);
		case  1: if (base->pgm.pg_v) {
				 freemem += size;
				 return(1);
			 }
			 break;
	 }

	/*
	 * Take pages from head of queue
	 */

	pfd = phead.pf_next;
	i = 0;
	while (i < size) {
		register pfd_t	*pfd_next;

		ASSERT (pfd != &phead);
		ASSERT(pfd->pf_flags&P_QUEUE);
		ASSERT(pfd->pf_use == 0);


		/* Delink page from free queue and set up pfd
		 */

		pfd_next = pfd->pf_next;
		pfd->pf_prev->pf_next = pfd_next;
		pfd_next->pf_prev = pfd->pf_prev;
		pfd->pf_next = NULL;
		pfd->pf_prev = NULL;
		if (pfd->pf_flags&P_HASH)
			premove(pfd);
		pfd->pf_blkno = BLKNULL;
		pfd->pf_use = 1;
		pfd->pf_flags = 0;
		pfd->pf_rawcnt = 0;
		rp->r_nvalid++;

		/*
		 * Insert in page table
		 */

		base->pgm.pg_pfn = pfdattopfn(pfd);;
		pg_clrmod(base);

		if (validate)
			pg_setvalid(base);
		
		i++;
		base++;
		pfd = pfd_next;
	}
	return(0);
}


/*
 * Shred page table and update accounting for swapped
 * and resident pages.
 *	rp	-> ptr to the region structure.
 *	pt	-> ptr to the first pte to free.
 *	dbd	-> ptr to disk block descriptor.
 *	size	-> nbr of pages to free.
 *
 * Called with mem_lock set and returns the same way.
 */

pfree(rp, pt, dbd, size)
reg_t		*rp;
register pde_t	*pt;
register dbd_t	*dbd;
int		size;
{
	register struct pfdat	*pfd;
	register int		k;
	extern short		pgrmv[];


	/* 
	 * Zap page table entries
	 */

	ASSERT(memlocked());

	for (k = 0; k < size; k++, pt++) {
		if (pt->pgm.pg_v) {
			pfd = pfntopfdat(pt->pgm.pg_pfn);

			/* Free pages that aren't being used
			 * by anyone else
			 */
			if (--pfd->pf_use == 0) {

				/* Pages that are associated with disk
				 * go to end of queue in hopes that they
				 * will be reused.  All others go to
				 * head of queue so they will be reused
				 * quickly.
				 */

				if (dbd == NULL || dbd->dbd_type == DBD_NONE) {
					/*
					 * put at head 
					 */
					pfd->pf_next = phead.pf_next;
					pfd->pf_prev = &phead;
					phead.pf_next = pfd;
					pfd->pf_next->pf_prev = pfd;
				} else {
					/*
					 * put at tail 
					 */
					pfd->pf_prev = phead.pf_prev;
					pfd->pf_next = &phead;
					phead.pf_prev = pfd;
					pfd->pf_prev->pf_next = pfd;
				}
				pfd->pf_flags |= P_QUEUE;
				freemem++;
			}

			if (rp != NULL)
				rp->r_nvalid--;
		}
		if (rp && dbd  &&  dbd->dbd_type == DBD_SWAP) {
			if (swfree1(dbd) == 0)
				pbremove(rp, dbd);
		}

		/*
		 * Change to zero pte's.
		 */

		pt->pgi.pg_pde = 0;
		if (dbd)
			dbd++->dbd_type = DBD_NONE;
	}
}




/*
 * Device number
 *	ip	-> inode pointer
 * returns:
 *	dev	-> device number
 */
effdev(ip)
register struct inode *ip;
{
	register int type;

	type = ip->i_ftype;
	if (type == IFREG || type == IFDIR) 
		return(ip->i_dev);
	return(ip->i_rdev);
}

/*
 * Find page by looking on hash chain
 *	dbd	-> Ptr to disk block descriptor being sought.
 * returns:
 *	0	-> can't find it
 *	pfd	-> ptr to pfdat entry
 */

struct pfdat *
pfind(rp, dbd)
register reg_t	*rp;
register dbd_t	*dbd;
{
	register dev_t		dev;
	register daddr_t	blkno;
	register pfd_t		*pfd;
	register long		inumber;

	/*	Hash on block and look for match.
	 */

	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		register inode_t	*ip;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		dev = effdev(ip);
		inumber = ip->i_number;

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if (rp->r_type == RT_PRIVATE)
			blkno = dbd->dbd_blkno + 1;
		else
			blkno = dbd->dbd_blkno;
		if (blkno == 0)
			return(NULL);
	}
	pfd = phash[blkno&phashmask];

	for (; pfd != NULL ; pfd = pfd->pf_hchain) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev == dev)
			&& (pfd->pf_inumber == inumber)) {
			if (pfd->pf_flags & P_BAD)
				continue;
			return(pfd);
		}
	}
	return(NULL);
}

/*
 * Insert page on hash chain
 *	dbd	-> ptr to disk block descriptor.
 *	pfd	-> ptr to pfdat entry.
 * returns:
 *	none
 */

pinsert(rp, dbd, pfd)
register reg_t	*rp;
register dbd_t	*dbd;
register pfd_t	*pfd;
{
	register dev_t	dev;
	register int	blkno;
	register long	inumber;

	/* Check page range, see if already on chain
	 */

	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		register inode_t	*ip;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		inumber = ip->i_number;
		dev = effdev(ip);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if (rp->r_type == RT_PRIVATE)
			blkno = dbd->dbd_blkno + 1;
		else
			blkno = dbd->dbd_blkno;

		/*
		 *	If blkno is zero, then we can't hash the page.
		 *	This happens for the last data page of a stripped
		 *	a.out that is an odd number of blocks long.
		 */

		if (blkno == 0)
			return;
	}

	ASSERT(pfd->pf_hchain == NULL);
	ASSERT(pfd->pf_use > 0);

	/*
	 * insert newcomers at tail of bucket
	 */

	{
		register struct pfdat *pfd1, *p;
		p = phash[blkno&phashmask]; 
		for (pfd1 = p; pfd1 ;
		    p=pfd1, pfd1 = pfd1->pf_hchain) {
			if ((pfd1->pf_blkno == blkno) &&
			   (pfd1->pf_dev == dev) &&
			   (pfd1->pf_inumber == inumber)) {
#if DEBUG == YES
				cmn_err(CE_CONT,
					"swapdev %x %x %x\n", swapdev,
					pfd1->pf_dev,dev);
				cmn_err(CE_CONT,
					"blkno %x %x\n",
					blkno, pfd1->pf_blkno);
				cmn_err(CE_CONT,
					"swpi %x %x\n", pfd1->pf_swpi,
					pfd->pf_swpi);
				cmn_err(CE_CONT,
					"pfd %x %x\n", pfd,pfd1);
				cmn_err(CE_CONT,
					"use %x %x\n",
					pfd->pf_use, pfd1->pf_use);
				cmn_err(CE_CONT,
					"flags %x %x\n", pfd->pf_flags,
					pfd1->pf_flags);
#endif
				cmn_err(CE_PANIC,
					"pinsert - pinsert dup");
			}
		}
		if (p == NULL)
			phash[blkno&phashmask] = pfd;
		else
			p->pf_hchain = pfd;
		pfd->pf_hchain = pfd1;
	}

	/*	Set up the pfdat.  Note that only swap pages are
	 *	put on the hash list for now.
	 */

	pfd->pf_dev = dev;
	pfd->pf_inumber = inumber;
	if (dbd->dbd_type == DBD_SWAP) {
		pfd->pf_swpi = dbd->dbd_swpi;
		pfd->pf_flags |= P_SWAP;
	} else {
		pfd->pf_flags &= ~P_SWAP;
	}
	pfd->pf_blkno = blkno;
	pfd->pf_flags |= P_HASH;
}


/*
 * remove page from hash chain
 *	pfd	-> page frame pointer
 * returns:
 *	0	Entry not found.
 *	1	Entry found and removed.
 */
premove(pfd)
register struct pfdat *pfd;
{
	register struct pfdat *pfd1, *p;
	int	rval;


	rval = 0;
	p = phash[pfd->pf_blkno&phashmask];
	for (pfd1 = p; pfd1 ;
	    p=pfd1, pfd1 = pfd1->pf_hchain) {
		if (pfd1 == pfd) {
			if (p == pfd1)
				phash[pfd->pf_blkno&phashmask] =
					pfd->pf_hchain;
			else
				p->pf_hchain = pfd->pf_hchain;
			rval = 1;
			break;
		}
	}
	/*
	 * Disassociate page from disk and
	 * remove from hash table
	 */
	pfd->pf_blkno = BLKNULL;
	pfd->pf_hchain = NULL;
	pfd->pf_flags &= ~P_HASH;
	pfd->pf_dev = 0;
	pfd->pf_inumber = 0;
	return(rval);
}

/*
 * Allocate system virtual address space and
 * allocate or link  pages.
 */
sptalloc(size, mode, base, flag)
register int size, mode, base;
int flag;
{
	register i, sp;

	/*
	 * Allocate system virtual address space
	 */
	if ((sp = malloc(sptmap, size))  ==  0) {

#ifdef DEBUG
		cmn_err(CE_WARN, "No kernel virtual space.");
		cmn_err(CE_CONT, "\tsize=%d, mode=%d, base=%d\n",
			size, mode, base);
#endif
		return(NULL);
	}
	/*
	 * Allocate and fill in pages
	 */
	if (base  ==  0) {
		reglock(&sysreg);
		memlock();
		if (ptmemall(&sysreg, kvtokptbl(ctob(sp)),
			size, 0, flag & NOSLEEP)) {
			memunlock();
			regrele(&sysreg);
			mfree(sptmap,size,sp);
			return(NULL);
		}
		memunlock();
		regrele(&sysreg);
	}

	/*
	 * Setup page table entries
	 */
	for (i = 0; i < size; i++) {
		if (base > 0)
			*(int *)kvtokptbl(ctob(sp + i)) =
				mkpde(mode, base++);
		else
			*(int *)kvtokptbl(ctob(sp + i)) |= mode;

		flushaddr(ctob(sp + i));
	}

	return(ctob(sp));
}

sptfree(vaddr, size, flag)
register int size;
{
	register i, sp;

	sp = btoc(vaddr);
	if (flag) {
		memlock();
		pfree(NULL, kvtokptbl(vaddr), NULL, size);
		memunlock();
	}
	for (i = 0; i < size; i++) {
		*(int *)kvtokptbl(ctob(sp + i)) = 0;
	}
	mfree(sptmap, size, sp);
}

/*
 * Initialize memory map
 *	first	-> first free page #
 *	last	-> last free page #
 * returns:
 *	none
 */

meminit(first, last)
register int first;
{
	register struct pfdat *pfd;
	register int i;

	freemem = (last - first);
	maxmem = freemem;
	availrmem = freemem;
	availsmem = freemem;

	/*
	 * Setup queue of pfdat structures.
	 * One for each page of available memory.
	 */

	pfd = pfntopfdat(first);
	phead.pf_next = &phead;
	phead.pf_prev = &phead;

	/*
	 * Add pages to queue, high memory at end of queue
	 * Pages added to queue FIFO
	 */

	for (i = freemem; --i >= 0; pfd++) {
		pfd->pf_next = &phead;
		pfd->pf_prev = phead.pf_prev;
		phead.pf_prev->pf_next = pfd;
		phead.pf_prev = pfd;
		pfd->pf_flags = P_QUEUE;
	}
}


/*
 * flush all pages associated with a mount device
 *	mp	-> mount table entry
 * returns:
 *	none
 */

punmount(mp)
register struct mount *mp;
{
	register pfd_t		*pfd;
	register pfd_t		*pfdlim;

	memlock();
	pfd = pfntopfdat(btoc(kpbase));
	pfdlim = pfntopfdat(maxclick);
	for (;  pfd < pfdlim  ;  pfd++) {
		if (mp->m_dev == pfd->pf_dev)
			if ((pfd->pf_flags & (P_HASH | P_SWAP)) == P_HASH)
				premove(pfd);
	}
	memunlock();
}

/*
 * Find page by looking on hash chain and remove it.
 *	dbd	Ptr to disk block descriptor for block to remove.
 * returns:
 *	0	-> can't find it.
 *	1	-> page found and removed.
 */
long pbrmcnt;

pbremove(rp, dbd)
reg_t	*rp;
dbd_t	*dbd;
{
	register struct pfdat	*pfd;
	register struct pfdat	*p;
	register int		blkno;
	register dev_t		dev;
	register long		inumber;

	/*
	 * Hash on block and look for match
	 */


	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		register inode_t	*ip;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

pbrmcnt++;
		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		dev = effdev(ip);
		inumber = ip->i_number;

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if (rp->r_type == RT_PRIVATE)
			blkno = dbd->dbd_blkno + 1;
		else
			blkno = dbd->dbd_blkno;
		if (blkno == 0)
			return(0);
	}

	for (pfd=p=phash[blkno&phashmask]; pfd ;p=pfd, pfd = pfd->pf_hchain) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev== dev)
			&& (pfd->pf_inumber == inumber)) {
			if (p!= pfd)
				p->pf_hchain = pfd->pf_hchain;
			else
				phash[blkno&phashmask] = pfd->pf_hchain;
			pfd->pf_blkno = BLKNULL;
			pfd->pf_hchain = NULL;
			pfd->pf_flags &= ~P_HASH;
			pfd->pf_dev = 0;
			pfd->pf_inumber = 0;
			return(1);
		}
	}

	return(0);
}



/*
 * Reserve size memory pages.  Returns with freemem
 * decremented by size.  Return values:
 *	0 - Memory available immediately
 *	1 - Had to sleep to get memory
 *     -1 - not enough memory and nosleep flag is set
 */

memreserve(rp, size, nosleep)
register reg_t *rp;
{
	register struct proc *p;

	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(memlocked());

	if (freemem >= size) {
		freemem -= size;
		return(0);
	}
	if (size > 1  &&  nosleep)
		return(-1);
	p = u.u_procp;
	while (freemem < size) {
		memunlock();
		regrele(rp);
		p->p_stat = SXBRK;
		++sxbrkcnt;
		if (runout) {
			runout = 0;
			wakeup(&runout);
		}
		swtch();
		reglock(rp);
		memlock();
	}
	freemem -= size;
	return(1);
}

flushpgch(ip)
register struct inode *ip;
{
	/*
	 * Flush the page cache of pages associated with the file
	 * from which this region was initialized. We do it here, rather
	 * than iput or some other file system  routine, because file
	 * system writers shouldn't have to bother with it.
	 */
	register int nblks, blkspp, i;
	register int bsize;
	struct region	reg;
	struct dbd	dbd;

	dbd.dbd_type = DBD_FILE;
	reg.r_iptr = ip;
	bsize = FSBSIZE(ip);
	nblks = (ip->i_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;
	nblks = ((nblks + blkspp - 1) / blkspp) * blkspp;
	memlock();
	for (i = 0  ;  i < nblks  ;  i += blkspp) {

		/*	Note the following grossness.  When we
		**	inserted these pages, we used either
		**	the first or the second block number
		**	to hash on depending on whether the
		**	page was private or shared.  Now we
		**	don't know which it is so we must do
		**	the page.  Note that the page which
		**	contains both text and data could
		**	be in the table twice so we must do
		**	both pbremove's even if the first
		**	one succeeds.
		*/
		
		dbd.dbd_blkno = i;
		reg.r_type = RT_PRIVATE;
		pbremove(&reg, &dbd);
		reg.r_type = RT_STEXT;
		pbremove(&reg, &dbd);
	}
	memunlock();
}
