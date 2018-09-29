/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/grow.c	10.9"
#include "sys/types.h"
#include "sys/bitmasks.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/sbd.h"

extern int userstack[];

/* brk and sbrk system calls
*/

sbreak()
{
	struct a {
		uint nva;
	};
	register preg_t	*prp;
	register reg_t	*rp;
	register uint	nva;
	register int	change;

	/*	Find the processes data region.
	 */

	prp = findpreg(u.u_procp, PT_DATA);
	if (prp == NULL)
		goto sbrk_err;
	rp = prp->p_reg;
	reglock(rp);

	nva = ((struct a *)u.u_ap)->nva;
	if ((nva > UVUBLK) || (nva < u.u_exdata.ux_datorg)) {
		regrele(rp);
		goto sbrk_err;
	}

	change = btoc(nva) - btoct(prp->p_regva) - rp->r_pgsz;
	if (change > 0  &&  chkpgrowth(change) < 0) {
		regrele(rp);
		goto sbrk_err;
	}

	if (growreg(prp, change, DBD_DZERO) < 0) {
		regrele(rp);
		return;
	}

	if (u.u_dmm){
		register preg_t	*dprp;

		dprp = findpreg(u.u_procp, PT_DMM);
		ASSERT(dprp != NULL);
		if (chkstbl(u.u_procp, dprp, rp->r_pgsz) < 0){
			growreg(prp, -change, DBD_DZERO);
			goto sbrk_err;
		}
		loadstbl(&u, dprp, 0);
	}

	regrele(rp);

	if (change != 0) {
		/*	Reload the mmu register for section 2.
		*/

		loadmmu(u.u_procp, SCN2);
	}

	return;

sbrk_err:
	u.u_error = ENOMEM;
	return;
}

/* grow the stack to include the SP
* true return if successful.
*/

grow(sp)
int *sp;
{
	register preg_t	*prp;
	register reg_t	*rp;
	register	si;

	/*	Find the processes stack region.
	 */

	prp = findpreg(u.u_procp, PT_STACK);
	if (prp == NULL)
		return(0);
	rp = prp->p_reg;
	reglock(rp);

	si = btoc((int)sp - (unsigned)userstack) - rp->r_pgsz + SINCR;
	if (si <= 0  ||  (uint)sp >= UVSHM) {
		regrele(rp);
		return(0);
	}

	if (chkpgrowth(si) < 0) {
		regrele(rp);
		return(0);
	}

	if (growreg(prp, si, DBD_DZERO) < 0) {
		regrele(rp);
		return(0);
	}

	u.u_ssize = rp->r_pgsz;
	u.u_pcb.sub = (int *)((unsigned)userstack + ctob(rp->r_pgsz));

	regrele(rp);

	/*	Reload the mmu registers for section 3.
	*/

	loadmmu(u.u_procp, SCN3);

	return(1);
}



/*
 * Allocate page tables.  Typically called to get
 * page tables for user process but may be called
 * for phys IO or to get DMA windows.  Returned address
 * is properly aligned for DMA.  The allocated page tables
 * are contiguous in kernel virtual address space .
 *
 *  npgtbls = number of (PTSIZE byte) page tables to allocate.
 *  If more than one physical pages are requested, they are mapped
 *  to sysreg for contiguous kernel virtual address.
 *  Otherwise, the page tables allocated are mapped to kernel virtual space
 *  using the macro phystokv(phys) = phys-page1+kv_base.
 */

pde_t *
uptalloc(npgtbls)
{
	pde_t *ptalloc();

	return(ptalloc(npgtbls,0));
}

/*
 * ptalloc(npgtbls,flag)
 *
 * flag = 0 		  -> same as uptalloc()
 * flag & PHYSCONTIG > 0  -> allocate physically contiguous pages
 * flag & NOSLEEP > 0	  -> return immediately if no memory
 */

pde_t *
ptalloc(npgtbls, flag)
{
	register int	nbytes;
	register int	pt;
	register pfd_t	*pf;
	register int	physaddr;
	register int	i;
	register int	j;

	if (npgtbls == 0)
		return(NULL);

	nbytes = npgtbls << PTSZSHFT;

	/*	If we are trying to allocate less than a full
	 *	page of page tables, then check the list of
	 *	pages which currently are being used for page
	 *	tables.
	 */
	
	if (npgtbls < NPTPP) {

		/*	Get mask with low npgtbls bits on.
		 */

		i = setmask[npgtbls];

		for (pf = ptfree.pf_next ; pf != &ptfree ;
		    pf = pf->pf_next) {

			/*	Try all positions of the mask to
			 *	find npgtbls contiguous page tables
			 *	in a page.
			 */

			for (j = 0  ;  j <= NPTPP - npgtbls  ;  j++) {
				if ((pf->pf_use & (i << j)) == (i << j)) {

					/* We have found the page
					 * tables.  Turn off their bits.
					 * If no page tables are left in
					 * the page, then remove page 
					 * from the page table list.
					 */

					pf->pf_use &= ~(i << j);
					if (pf->pf_use == 0) {
						pf->pf_prev->pf_next =
							pf->pf_next;
						pf->pf_next->pf_prev =
							pf->pf_prev;
						pf->pf_next = 0;
						pf->pf_prev = 0;
					}

					/* Get address of page tables we
					 * have allocated.  Update the
					 * free page table count and
					 * clear the page tables.
					 */

					physaddr = ctob(pfdattopfn(pf));
					pt = phystokv(physaddr);
					pt += j << PTSZSHFT;
					nptfree -= npgtbls;
					bzero((caddr_t)pt, nbytes);
					return((pde_t *)pt);
				}
			}
		}
	}

	/*	We could not allocate the required number of 
	 *	contiguous page tables from a single page 
	 *	of page tables on the free list.
	 *	Allocate some more kernel virtual address
	 *	space and physical memory for page tables.
	 *	Allocate an integral number of pages
	 *	to use for page tables.
	 */

	i = pttopgs(npgtbls);
	if (availrmem - i < tune.t_minarmem  ||
	   availsmem - i < tune.t_minasmem) {
		nomemmsg("ptalloc", i, 0, 0);
		return(NULL);
	}
	availrmem -= i;
	availsmem -= i;

	pt =  getcpages(i, flag & NOSLEEP);

	if (pt <= 0) {
		availrmem += i;
		availsmem += i;

		/*	A message has been printed by getcpages.
		*/

		return(NULL);
	}

	ASSERT((pt & POFFMASK) == 0);
	nptalloced += i << NPTPPSHFT;
	nptfree += i << NPTPPSHFT;

	/*	Clear the free page table bit masks for all
	 *	of the pages which we have just allocated.
	 */
	
	for (j = 0  ;  j < i  ;  j++) {
		pf = kvtopfdat(pt + ctob(j));
		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);
		pf->pf_use = 0;
	}

	/*	Add any unused page tables at the end of the
	 *	last page to the free list.
	 */
	

	j = npgtbls % NPTPP;
	if (j > 0) {
		pf = kvtopfdat(pt + ctob(i-1));
		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);
		pf->pf_use = setmask[NPTPP - j] << j;
		pf->pf_next = ptfree.pf_next;
		pf->pf_prev = &ptfree;
		ptfree.pf_next->pf_prev = pf;
		ptfree.pf_next = pf;
	}

	/*	Update the count of the number of free page tables
	 *	and zero out the page tables we have just allocated.
	 */

	nptfree -= npgtbls;
	bzero((caddr_t)pt, nbytes);

	return((pde_t *)pt);
}

/*
 * Free previously allocated page tables
 */

uptfree(pt, npgtbls)
register int	pt;
register int	npgtbls;
{
	register int	nfree;
	register int	ndx;
	register pfd_t	*pf;
	int		pfn;

	if (npgtbls == 0)
		return;

	/*	Free the page tables and update the count of
	 *	free page tables.
	 */

	while (npgtbls > 0) {

		/* Get a pointer to the pfdat for the page in which
		 * we are freeing page tables.  Compute the index
		 * into the page of the first page table being freed.
		 */

		pfn = svtopfn(pt);
		pf = kvtopfdat(pt);
		ndx = (pt - ctob(btoct(pt))) >> PTSZSHFT;

		ASSERT((pf->pf_flags & (P_QUEUE | P_HASH)) == 0);

		/* Compute the number of page tables in this page
		 * which we are freeing.
		 */
		
		nfree = min(npgtbls, NPTPP);
		if (((ctob(btoc(pt + 1)) - pt) >> PTSZSHFT) < nfree)
			nfree = (ctob(btoc(pt)) - pt) >> PTSZSHFT;

		/*	If the current page has no free page tables
		 *	and we are not going to free the entire
		 *	page, then put the pfdat on the free page
		 *	table list.
		 */
		
		if (pf->pf_use == 0  &&  nfree < NPTPP) {
			ASSERT(pf->pf_next == NULL);
			ASSERT(pf->pf_prev == NULL);
			pf->pf_next = ptfree.pf_next;
			pf->pf_prev = &ptfree;
			ptfree.pf_next->pf_prev = pf;
			ptfree.pf_next = pf;
		}

		/*	Set the appropriate bits in the pfdat.  If
		 *	we free the entire page, then return it to
		 *	the system free space list and update the
		 *	physical to system virtual address map to
		 *	show that the page is no longer being used
		 *	for page tables.
		 */
		
		ASSERT((pf->pf_use & (setmask[nfree] << ndx)) == 0);
		pf->pf_use |= setmask[nfree] << ndx;
		if (pf->pf_use == setmask[NPTPP]) {

			/* Remove pfdat from page table allocation
			 * list and reset use count.
			 */

			if (nfree != NPTPP) {
				ASSERT(pf->pf_next != NULL);
				ASSERT(pf->pf_prev != NULL);
				pf->pf_prev->pf_next = pf->pf_next;
				pf->pf_next->pf_prev = pf->pf_prev;
			}
			pf->pf_use = 1;


			/* Free the pages
			 * Update the count of allocated
			 * and free pages.
			 */

			freepage(pfn);
			availrmem += 1;
			availsmem += 1;
	
			nptalloced -= NPTPP;
			nptfree -= NPTPP - nfree;
		} else
			nptfree += nfree;

		npgtbls -= nfree;
		pt += nfree << PTSZSHFT;
	}
}

/*	Check that a process is not trying to expand
**	beyond the maximum allowed virtual address
**	space size.
*/

chkpgrowth(size)
register int	size;	/* Increment being added (in pages).	*/
{
	register preg_t	*prp;
	register reg_t	*rp;

	prp = u.u_procp->p_region;

	while (rp = prp->p_reg) {
		size += rp->r_pgsz;
		prp++;
	}

	if (size > tune.t_maxumem)
		return(-1);
	
	return(0);
}

