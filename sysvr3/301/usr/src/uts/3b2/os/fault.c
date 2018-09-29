/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/fault.c	10.9"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/reg.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fstyp.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/utsname.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/swap.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"


/*	Protection fault handler
 */

pfault(vaddr, pd)
register pde_t	*pd;	/* Virtual address of faulting pde.	*/
{
	register dbd_t	*dbd;
	register proc_t	*p;
	register pfd_t	*pfd;
	register reg_t	*rp;
	pde_t		pttmp;


	/*	Get a pointer to the region which the faulting
	 *	virtual address is in.
	 */

	p = u.u_procp;
	if ((rp = findreg(p, vaddr))==NULL)
		return(SIGSEGV);

	/*	Check to see that the pde hasn't been modified
	 *	while waiting for the lock
	 */

	if (!pd->pgm.pg_v) {
		regrele(rp);
		return(0);
	}

	/*	Now check for a real protection error as opposed
	 *	to a copy on write.
	 *	Also check if shared text. This is part of
	 *	the mau problem of performing operations
	 *	on shared text. If it does, a protection
	 *	fault is generated since the copy on
	 *	write bit had been set for shared text.
	 */

	minfo.pfault++;
	if (!pd->pgm.pg_cw || rp->r_type == RT_STEXT) {
		regrele(rp);
		return(SIGBUS);
	}

	ASSERT(rp->r_type == RT_PRIVATE);

	dbd = (dbd_t *)pd + NPGPT;
	pfd = pfntopfdat(pd->pgm.pg_pfn);

	/*	Copy on write
	 *	If use is 1, and page is not from a file,
	 *	steal it, otherwise copy it
	 */

	memlock();

	if (pfd->pf_use > 1  ||  dbd->dbd_type == DBD_FILE
	   ||  dbd->dbd_type == DBD_LSTFILE) {
		minfo.cw++;
		pttmp.pgi.pg_pde = 0;

		/*	We are locking the page we faulted on
		**	before calling ptmemall because
		**	ptmemall may unlock the region.  If
		**	he does, then the page could be stolen
		**	and we would be copying incorrect
		**	data into our new page.
		*/

		pg_setlock(pd);
		pfntopfdat(pd->pgm.pg_pfn)->pf_rawcnt++;
		ptmemall(rp, &pttmp, 1, 1, 0);

		/*	Its O.K. to unlock the page now since
		**	ptmemall has locked the region again.
		*/

		ASSERT(rp->r_flags & RG_LOCK);
		ASSERT(pd->pgm.pg_lock);
		ASSERT(pfntopfdat(pd->pgm.pg_pfn)->pf_rawcnt > 0);

		if (--pfntopfdat(pd->pgm.pg_pfn)->pf_rawcnt == 0)
			pg_clrlock(pd);
		memunlock();
		bcopy(phystokv(pd->pgm.pg_pfn<<PNUMSHFT),
			phystokv(pttmp.pgm.pg_pfn<<PNUMSHFT), NBPP);
		memlock();
		pfree(rp, pd, dbd, 1);
		*pd = pttmp;
	} else {

		/*	We are modifiying the page so
		 *	break the disk association to swap.
		 */
		
		if (pfd->pf_flags & P_HASH)
			premove(pfd);

		if (dbd->dbd_type == DBD_SWAP)
			swfree1(dbd);
		dbd->dbd_type = DBD_NONE;
		minfo.steal++;
	}

	memunlock();

	/*	Set the modify bit here before the region is unlocked
	 *	so that getpages will write the page to swap if necessary.
	 */

	pg_setmod(pd);
	pg_clrcw(pd);
	pg_setprot(pd, PTE_RW);

	flushaddr(vaddr);

	regrele(rp);

	return(0);
}

/*	Translation fault handler
 */

vfault(vaddr, pd)
register pde_t	*pd;	/* Virtual address of faulting pde.	*/
{
	register proc_t	*p;
	register dbd_t	*dbd;
	register pfd_t	*pfd;
	register reg_t	*rp;
	register inode_t *ip;

	ASSERT(u.u_procp->p_flag & SLOAD);

	dbd = (dbd_t *)pd + NPGPT;

	/*	Lock the region containing the page that faulted.
	 */

	p = u.u_procp;
	if ((rp = findreg(p, vaddr))==NULL)
		return(SIGSEGV);

	/*	Check for an unassigned page.  This is a real
	 *	error.
	 */

	if (dbd->dbd_type == DBD_NONE) {
		regrele(rp);
		return(SIGSEGV);
	}

	/*	Check that the page has not been read in by
	 *	another process while we were waiting for
	 *	it on the reglock above.
	 */

	if (pd->pgm.pg_v) {
		regrele(rp);
		return(0);
	}
	minfo.vfault++;

	/*	Allocate a page in case we need it.  We must
	 *	do it now because it is not convenient to
	 *	wait later if no memory is available.  If
	 *	ptmemall does a wait and some other process
	 *	allocates the page first, then we have
	 *	nothing to do.
	 */
	
	memlock();
	if (ptmemall(rp, pd, 1, 0, 0)) {
		ASSERT(pg_isvalid(pd));
		memunlock();
		regrele(rp);
		return(0);
	}

	/*	See what state the page is in.
	 */

	switch (dbd->dbd_type) {
	case DBD_DFILL:
	case DBD_DZERO:{

		/* 	Demand zero or demand fill page.
		 */

		minfo.demand++;
		memunlock();
		if (dbd->dbd_type == DBD_DZERO)
			bzero(phystokv(pd->pgm.pg_pfn<<PNUMSHFT), NBPP);
		dbd->dbd_type = DBD_NONE;
		break;
	}
	case DBD_SWAP:
	case DBD_FILE:
	case DBD_LSTFILE:{

		/*	Page is on swap or in a file.  See if a
		 *	copy is in the hash table.
		 */

		if (pfd = pfind(rp, dbd)) {
			pde_t	pde_save;

			/*	Page is in cache.
			 *	If it is also on the free list,
			 *	remove it.
			 */

			ASSERT(memlocked());

			minfo.cache++;
			if (pfd->pf_flags&P_QUEUE) {
				ASSERT(pfd->pf_use == 0);
				ASSERT(freemem > 0);
				freemem--;
				pfd->pf_flags &= ~P_QUEUE;
				pfd->pf_prev->pf_next = pfd->pf_next;
				pfd->pf_next->pf_prev = pfd->pf_prev;
				pfd->pf_next = NULL;
				pfd->pf_prev = NULL;
			}

			/*	Free the page we allocated above
			 *	since we don't need it.
			 */

			pde_save = *pd;
			pg_setvalid(&pde_save);
			pfree(rp, &pde_save, NULL, 1);
			rp->r_nvalid++;
			pfd->pf_use++;
			pd->pgm.pg_pfn = pfdattopfn(pfd);

			/*	If the page has not yet been read
			 *	in from swap or file, then wait for
			 *	the I/O to complete.
			 */

			while (!(pfd->pf_flags & P_DONE)) {
				pfd->pf_waitcnt++;
				memunlock();
				sleep(pfd, PZERO);
				memlock();
				if (pfd->pf_flags & P_BAD) {
					memunlock();
					regrele(rp);
					return(SIGKILL);
				}
			}
			memunlock();
		} else {

			/*	Must read from swap or a file.
			 *	Get the pfdat for the newly allocated
			 *	page and insert it in the hash table.
			 *	Note that it cannot already be there
			 *	because the pfind above failed and
			 *	mem_lock is still locked.
			 */
			
			pfd = pfntopfdat(pd->pgm.pg_pfn);
			ASSERT((pfd->pf_flags & P_HASH) == 0);

			/*	Don't insert in hash table if this
			 *	block is from a swap file we are
			 *	trying to delete.
			 */

			if (dbd->dbd_type == DBD_SWAP) {
				register int	swapdel;
				pglst_t		pglist;

				swapdel = swaptab[dbd->dbd_swpi].st_flags
						& ST_INDEL;
				pinsert(rp, dbd, pfd);
				memunlock();

				/*	Read from swap.
				 */

				minfo.swap++;
				pglist.gp_ptptr = pd;
				swap(&pglist, 1, B_READ);
				if (swapdel) {
					if (swfree1(dbd) == 0)
						premove(pfd);
					dbd->dbd_type = DBD_NONE;
				}
			} else {
				/*	Read from file
				 */
				int vaddr;
				int offset;
				int retval;
				int i;

				minfo.file++;
				pinsert(rp, dbd, pfd);
				memunlock();
				vaddr = phystokv(ctob(pd->pgm.pg_pfn));
				ip = rp->r_iptr;
				offset = dbd->dbd_blkno*FSBSIZE(ip);
				retval = FS_READMAP(ip, offset, NBPP, vaddr, 1);
				if (retval <= 0) {
					killpage(rp, pd);
					regrele(rp);
					return(SIGKILL);
				}
				/*
				 * clear the last bytes of a partial page
				 */
				if (dbd->dbd_type == DBD_LSTFILE) {
					i = poff(rp->r_filesz);
					if (i > 0)
						bzeroba(vaddr+i, NBPP-i);
				}
			}

			/*	Mark the I/O done in the pfdat and
			 *	awaken anyone who is waiting for it.
			 */

			memlock();
			pfd->pf_flags |= P_DONE;
			if (pfd->pf_waitcnt) {
				pfd->pf_waitcnt = 0;
				wakeup(pfd);
			}
			memunlock();
		}
		break;
	}
	default:
		cmn_err(CE_PANIC, "vfault - bad dbd_type");
	}

	pg_setvalid(pd);
	pg_clrmod(pd);
	regrele(rp);

	return(0);
}


/*
 * Clean up after a read error during vfault processing.
 * This code frees the previously allocated page, and marks
 * the pfdat as bad.  It leaves the pde, and dbd in their original
 * state.  It assumes the pde is presently invalid.
 */
killpage(rp, pd)
reg_t *rp;
register pde_t *pd;
{
	register pde_t save_pde;
	register pfd_t *pfd;

	save_pde = *pd;
	pg_setvalid(pd);
	memlock();
	pfd = pfntopfdat(pd->pgm.pg_pfn);
	pfd->pf_flags |= P_BAD|P_DONE;
	if (pfd->pf_flags & P_HASH)
		premove(pfd);
	if (pfd->pf_waitcnt) {
		pfd->pf_waitcnt = 0;
		wakeup(pfd);
	}
	pfree(rp, pd, NULL, 1);
	memunlock();
	*pd = save_pde;
}

