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
/* @(#)fault.c	1.6 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psl.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/buf.h"
#include "sys/utsname.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#include "sys/debug.h"

extern int freemem;

#if PFLOGON == 1
#define PFLGSZ 256
struct pfdlog {
	char type;	/* 3=memfree; 2=vfault; 1=allocate; 0=pfree */
	char pfuse;
	short pfnum;
	unsigned int process;
	struct region *regp;
} pfdlog[];
extern int pflgndx;
#endif

/*
 * Protection fault handler
 *	vaddr	-> virtual address of fault
 *	pt	-> page table address
 */
pfault(vaddr, pt)
register pte_t	*pt;
{
	register pfd_t	*pfd;
	register reg_t	*rp;
	register dbd_t	*dbd;
	preg_t	*prp;
	pte_t	pttmp;
	int	tmp;
	int	oldpri;

	/*	Check that the fault is due to a copy on write page
	 */
	
	if (!pt->pgm.pg_cw)
		return(0);
	
	oldpri = curpri;	/* Save priority for later restoral */

	/*	Get a pointer to the region which the faulting
	 *	virtual address is in.
	 */

	prp = findreg(u.u_procp, vaddr);
	rp = prp->p_reg;
	reglock(rp);

	minfo.pfault++;

	/*	Find dbd as mapped by region
	 */

	tmp = btotp(vaddr - (int)prp->p_regva);
	dbd = (dbd_t *)&(rp->r_list[tmp/NPGPT][tmp%NPGPT]) + NPGPT;
	pfd = pfdat + pt->pgm.pg_pfn;

	/*	Copy on write
	 *	If use is > 1, or page is associated with a file, copy it
	 */
	if(pfd->pf_use > 1 || dbd->dbd_type == DBD_FILE) {
		minfo.cw++;
		pttmp.pgi.pg_pte = 0;
		tmp = pt->pgm.pg_lock;
		pt->pgm.pg_lock = 1;
		ptmemall(rp, &pttmp, 1, 1);
		pt->pgm.pg_lock = tmp;
		copypage(pt->pgi.pg_pte, pttmp.pgi.pg_pte);
		pfree(rp, pt, dbd, 1);
		*pt = pttmp;
	} else {
		/*	Break the disk association of the page
		 */
		minfo.steal++;
		if (pfd->pf_flags & P_HASH)
			premove(pfd);
		if (dbd->dbd_type == DBD_SWAP)
			swfree1(dbd);
		dbd->dbd_type = DBD_NONE;
	}

	/*	Make page table entry writeable and
	 *	clear translation buffer
	 */

	pt->pgm.pg_cw = 0;
	if (rp->r_type != RT_STEXT)
		pt->pgm.pg_prot = PTE_UW; 

	pt->pgi.pg_pte |= (PG_V | PG_REF | PG_NDREF | PG_M);
	pt->pgi.pg_pte2 = PG_PE2(pt->pgi.pg_pte);
	invsatb(USRATB, vaddr);
	regrele(rp);

	u.u_procp->p_pri = curpri = oldpri;
	return(1);
}


/*
 * Translation fault handler
 *	vaddr	-> virtual address
 *	pt	-> page table address
 */
vfault(vaddr, pt)
register pte_t	*pt;
{
	register reg_t	*rp;
	preg_t	*prp;
	dbd_t	*dbd;
	int	tmp;
	int	oldpri;

	/*	Check for an unassigned page.  This is a real error.
	 */

	if (pt->pgi.pg_pte == 0)
		return(0);

	minfo.vfault++;

	oldpri = curpri;	/* Save CPU priority for later restoral */

	/*	Lock the region containing the page that faulted.
	 */

	prp = findreg(u.u_procp, vaddr);
	rp = prp->p_reg;
	reglock(rp);

	/*	Check that the page has not been read in by
	 *	another process while we were waiting for
	 *	it on the reglock above.
	 */

	if (pt->pgm.pg_v) {
		regrele(rp);
		curpri = u.u_procp->p_pri = oldpri;
		return(1);
	}

	/*	Find pte and dbd as mapped by region
	 */

	tmp = btotp(vaddr - (int)prp->p_regva);
	pt = &rp->r_list[tmp/NPGPT][tmp%NPGPT];
	dbd = (dbd_t *)pt + NPGPT;

	/*	See what state the page is in.
	 */

	switch (dbd->dbd_type) {
	case DBD_DFILL:
	case DBD_DZERO:

		/* 	Demand zero or demand fill page.
		 */

		minfo.demand++;

		pt->pgi.pg_pte &= ~PG_CW;
		if (ptmemall(rp, pt, 1, 0)) {
			regrele(rp);
			curpri = u.u_procp->p_pri = oldpri;
			return(1);
		}
		if (dbd->dbd_type == DBD_DZERO)
			clearpage(pt->pgi.pg_pte);
		dbd->dbd_type = DBD_NONE;
		pt->pgi.pg_pte |= (PG_V | PG_REF | PG_NDREF);
		pt->pgi.pg_pte2 = PG_PE2(pt->pgi.pg_pte);
		break;

	case DBD_SWAP:
	case DBD_FILE: {
		register pte_t	*ptep;
		register dbd_t	*dbdp;
		register 	i;
		register	count;
		struct pfdat	*pfd;
		pte_t		*endreg;
		int	type;
		int	swpi;
		int	maxprepage;
		int	curblk;
		int	nextblk;

		/*	If the page we want is in memory already, take it
		 */

		if (pageincache(rp, pt, dbd)) {
			minfo.cache++;
			break;
		}

		/*	Otherwise, get page(s) from disk. First reserve memory
		 *	for the ptfill below.  We must do this now to prevent
		 *	pinsert dups.
		 */

		if (memreserve(rp, tune.t_prep + 1)) {
			/*	We went to sleep waiting for memory.
			 *	Check if the page we're after got loaded in
			 *	the mean time.  If so, give back the memory
			 *	and return
			 */
			if (pt->pgi.pg_pte & (PG_V | PG_REF | PG_NDREF)) {
				freemem += tune.t_prep + 1;
				break;
			}
			if (pageincache(rp, pt, dbd)) {
				minfo.cache++;
				freemem += tune.t_prep + 1;
				break;
			}
		}

		/*	Scan ptes and dbds looking for a run of
		 *	contiguous pages to load from disk
		 */

		tmp = btots(vaddr - (int)prp->p_regva);
		if (stopg(tmp+1) < rp->r_pgoff + rp->r_pgsz)
			endreg = &rp->r_list[tmp][NPGPT];
		else {
			i = rp->r_pgoff + rp->r_pgsz - stopg(tmp);
			endreg = &rp->r_list[tmp][i];
		}

		type = dbd->dbd_type;
		swpi = dbd->dbd_swpi;
		maxprepage = tune.t_prep;

		count = 1;
		ptep = pt + 1;
		dbdp = dbd + 1;
		curblk = dbd->dbd_blkno;

		while ( (ptep < endreg) &&
			!(ptep->pgi.pg_pte & (PG_V|PG_REF|PG_NDREF)) &&
			(count < maxprepage) &&
			(dbdp->dbd_type == type) &&
			!(pfind(rp, dbdp)) ) {

			if (type == DBD_SWAP) {
				if ( (dbdp->dbd_swpi != swpi) ||
				     (dbdp->dbd_blkno != curblk+1) ) 
					break;
			}

			count++;
			ptep++;
			dbdp++;
			curblk++;
		}

		/*	Give back excess memory we're holding and fill
		 *	in page tables with real pages.
		 */
		ASSERT(tune.t_prep + 1 >= count);
		freemem += (tune.t_prep + 1 - count);
		rp->r_nvalid += count;
		ptfill(pt, count, rp->r_type, 0);

		/*
		 *	We now have a series of ptes with page frames
		 *	assigned, whose rightful contents are not in the
		 *	page cache. Pinsert the page frames, and read from
		 *	disk.
		 */
		
		vaddr &= ~(NBPP-1);

		if (type == DBD_SWAP) {
			int swapdel;

			minfo.swap++;

			swapdel = swaptab[swpi].st_flags & ST_INDEL;
			if (swapdel == 0) {
				ptep = pt;
				dbdp = dbd;
				for (i=0; i<count; i++) {
					pinsert(rp, dbdp,
						&pfdat[ptep->pgm.pg_pfn]);
					ptep++;
					dbdp++;
				}
			}

			swap(dbd, pt, vaddr, count, B_READ);

			ptep = pt;
			dbdp = dbd;
			for (i=0; i<count; i++) {
				if (swapdel) {
					swfree1(dbdp);
					dbdp->dbd_type = DBD_NONE;
				}

				/* 	Mark the I/O done, and awaken anyone
				 *	waiting for pfdats
				 */
				pfd = &pfdat[ptep->pgm.pg_pfn];
				pfd->pf_flags |= P_DONE;
				if (pfd->pf_flags & P_WANT) {
					pfd->pf_flags &= ~P_WANT;
					wakeup(pfd);
				}
				ptep->pgi.pg_pte |= (PG_V | PG_REF | PG_NDREF);
				ptep->pgi.pg_pte &= ~PG_M;
				ptep->pgi.pg_pte2 = PG_PE2(ptep->pgi.pg_pte);

				ptep++;
				dbdp++;
			}
		
		} else {	/* DBD_FILE */
			dev_t	edev;
			struct dbd	*tmpdbd, *enddbd;
			struct buf	*bp = NULL;
			caddr_t	bufadr;
			short	saveprot;

			/*
			 *	We're reading from a file.
			 *	Pinsert the page frames, except for the
			 *	last frame of the mapped region. This
			 *	frame will eventually be partially cleared
			 *	so it will not match the disk, and should
			 *	not be cleared
			 */
			 minfo.file++;

			 edev = rp->r_iptr->i_dev;
			 ptep = pt;
			 dbdp = dbd;

			 for (i=0; i<count; i++)
				if (dbdp->dbd_adjunct & DBD_LAST) break;
				else {
					pinsert(rp, dbdp,
						&pfdat[ptep->pgm.pg_pfn]);
					ptep++;
					dbdp++;
				}
			
			/* Read the pages from the file */

			enddbd = (struct dbd *)endreg + NPGPT;
			ptep = pt;
			dbdp = dbd;

			while (count > 0) {
				curblk = dbdp->dbd_blkno;
				tmpdbd = dbdp + 1;
				nextblk = curblk;
				while ( (tmpdbd < enddbd) &&
					(tmpdbd->dbd_type == DBD_FILE)) {
					if (tmpdbd->dbd_blkno != curblk) {
						nextblk = tmpdbd->dbd_blkno;
						break;
					}
					tmpdbd++;
				}

				ASSERT (nextblk != 0);
				if (bp) {
					brelse(bp);
				}
				if (curblk == nextblk) {
					bp = bread(edev, curblk);
				} else {
					bp = breada(edev, curblk, nextblk);
				}

				if (bp->b_flags & B_ERROR) {
					prdev("page read error", edev);
					brelse(bp);
					while (count > 0) {
						killpage(rp, ptep);
						ptep++;
						count--;
					}
					regrele(rp);
					curpri = u.u_procp->p_pri = oldpri;
					return(-1);
				}
				bufadr = bp->b_un.b_addr;

				while ( (dbdp < enddbd) &&
					(dbdp->dbd_blkno == curblk) &&
					(count > 0) ) {
					ASSERT(dbdp->dbd_type == DBD_FILE);
					ptep->pgi.pg_pte |=
							(PG_V|PG_REF|PG_NDREF);
					ptep->pgi.pg_pte2 |= PG_V;

					/* we have to set the protections
					 * for copyout and uclear
					 */
					saveprot = ptep->pgm.pg_prot;
					ptep->pgm.pg_prot = PTE_UW;
					ASSERT(bufadr < bp->b_un.b_addr + FsBSIZE(edev) );
					copyout(bufadr, vaddr, NBPP);
					if (dbdp->dbd_adjunct & DBD_LAST) {
						i = poff(rp->r_filesz);
						if(i > 0)
							upgclr(vaddr+i);
					}
					ptep->pgm.pg_prot = saveprot;
					ptep->pgi.pg_pte &= ~PG_M;
					ptep->pgi.pg_pte2 &= ~PG_M;
					invsatb(USRATB, vaddr);

					/*	wakeup anyone waiting for the
					 *	page frame
					 */
					pfd = &pfdat[ptep->pgm.pg_pfn];
					pfd->pf_flags |= P_DONE;
					if (pfd->pf_flags & P_WANT) {
						pfd->pf_flags &= ~P_WANT;
						wakeup(pfd);
					}

					ptep++;
					dbdp++;
					count--;
					vaddr += NBPP;
					bufadr += NBPP;
				}
			}
			if (bp) {
				brelse(bp);
			}
		}

		break;
	}

	case DBD_NONE:  
		regrele(rp);
		curpri = u.u_procp->p_pri = oldpri;
		return(0);
	default:
		panic("vfault - bad dbd_type");
	}

	regrele(rp);

	curpri = u.u_procp->p_pri = oldpri;
	return(1);
}

/*
 * Check if the page described by dbd is in memory.
 * If it is, latch onto it.  Return values:
 *	0 - page no where in sight.
 *	1 - got page
 */
pageincache(rp, pt, dbd)
register reg_t *rp;
register pte_t *pt;
register dbd_t *dbd;
{
	register pfd_t *pfd;

	/*	Look in page cache
	 */
	if (pfd = pfind(rp, dbd)) {

		/*	We found it.  If the page is on the freelist,
		 *	remove it.  If freemem is zero, someone already
		 *	has reserved the page, and we cannot use it.
		 */
		if (pfd->pf_flags & P_QUEUE) {
			ASSERT(pfd->pf_use == 0);
			ASSERT(pfd->pf_flags & P_DONE);
			if(freemem <= 0) {
				premove(pfd);
				return(0);
			}
			freemem--;
			pfd->pf_use = 1;
			pfd->pf_flags &= ~P_QUEUE;
			pfd->pf_prev->pf_next = pfd->pf_next;
			pfd->pf_next->pf_prev = pfd->pf_prev;
			pfd->pf_next = NULL;
			pfd->pf_prev = NULL;
		} else {

			/*	Wait for the i/o to complete.  If bad is set,
			 *	after waking, return failure.
			 */

			pfd->pf_use++;
			while ((pfd->pf_flags & P_DONE) == 0) {
				pfd->pf_flags |= P_WANT;
				sleep(pfd, PMEM);
			}
			if (pfd->pf_flags & P_BAD) {
				pfd->pf_use--;
				return(0);
			}
		}

		rp->r_nvalid++;
		pt->pgm.pg_pfn = pfd - pfdat;
		if (rp->r_type == RT_STEXT)
			pt->pgm.pg_prot = PTE_UR;

		pt->pgi.pg_pte |= (PG_V | PG_REF | PG_NDREF);
		pt->pgi.pg_pte &= ~PG_M;
		pt->pgi.pg_pte2 = PG_PE2(pt->pgi.pg_pte);
		return(1);
	}
	return(0);
}

/*
 * Clean up after a read error during vfault processing.
 * This code frees the previously allocated page, and marks
 * the pfdat as bad.  It leaves the pte, and dbd in their original
 * state.  It assumes the pte is presently invalid.
 */
killpage(rp, pt)
reg_t *rp;
register pte_t *pt;
{
	register pte_t save_pte;
	register pfd_t *pfd;

	save_pte = *pt;
	pt->pgm.pg_v = 1;
	pfd = &pfdat[pt->pgm.pg_pfn];
	pfd->pf_flags |= P_BAD|P_DONE;
	if (pfd->pf_flags & P_WANT) {
		pfd->pf_flags &= ~P_WANT;
		wakeup((caddr_t)pfd);
	}
	pfree(rp, pt, NULL, 1);
	*pt = save_pte;
}
