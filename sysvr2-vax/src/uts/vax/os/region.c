/* @(#)region.c	1.15 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/seg.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/buf.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/mtpr.h"
#include "sys/map.h"

#define	P0SPACE 0x40000000

reg_t	nullregion;
preg_t	nullpregion;

extern int sbittab[];
extern int maxupts;

void reginit()
{
	register reg_t *rp;

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


#if LOCKLOGON == 1
#define	LOCKLGSZ	64
struct locklog {
	uint	lock;
	uint	process;
	uint	region;
	uint	retva;
} locklog[LOCKLGSZ];
int lockndx=0;
#endif

extern int p0init_done;

reglock(rp)
register reg_t *rp;
{
	int	frame;

#	if LOCKLOGON == 1
		if (p0init_done) {
		locklog[lockndx].lock = 1;
		locklog[lockndx].process = (int)u.u_procp;
		locklog[lockndx].region = (int)rp;
		locklog[lockndx].retva = (&frame)[5];
		lockndx++;
		if (lockndx >= LOCKLGSZ) lockndx = 0;
		}
#	endif

	while (rp->r_flags & RG_LOCK) {
		rp->r_flags |= RG_WANTED;
		sleep((caddr_t)rp, PZERO);
	}
	rp->r_flags |= RG_LOCK;
}

regrele(rp)
register reg_t *rp;
{
	int	frame;

#	if LOCKLOGON == 1
		if (p0init_done) {
		locklog[lockndx].lock = 0;
		locklog[lockndx].process = (uint)u.u_procp;
		locklog[lockndx].region = (uint)rp;
		locklog[lockndx].retva = (&frame)[5];
		lockndx++;
		if (lockndx >= LOCKLGSZ) lockndx = 0;
		}
#	endif

	rp->r_flags &= ~RG_LOCK;
	if (rp->r_flags & RG_WANTED) {
		rp->r_flags &= ~RG_WANTED;
		wakeup((caddr_t)rp);
	}
}

/*
 * Allocate a new region.
 * Returns a locked region pointer or NULL on failure
 * The region is linked into the active list.
 */

reg_t *
allocreg(ip, type)
register struct inode	*ip;
{
	register reg_t *rp;

	if ((rp = rfree.r_forw) == &rfree) {
		printf("Region table overflow\n");
		u.u_error = ENOMEM;
		return(NULL);
	}
	/*
	 * Remove from free list
	 */
	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	/*	Initialize region fields and bump inode reference
	 *	count.
	 */

	rp->r_incore = 0;
	rp->r_type = type;
	rp->r_iptr = ip;
	rp->r_flags = RG_LOCK;
	if(ip != NULL)
		ip->i_count++;

	if(type != RT_PRIVATE) {
		/*
		 * Link onto active list
		 */
		rp->r_forw = ractive.r_forw;
		rp->r_back = &ractive;
		ractive.r_forw->r_back = rp;
		ractive.r_forw = rp;
	}

	return(rp);
}

/*
 * Free an unused region table entry.
 */
void
freereg(rp)
register reg_t *rp;	/* pointer to a locked region */
{
	register struct inode	*ip;
	register int		i;
	register int		lim;
	register int		size;
	register pte_t		*pt;
	register int		tsize;

	ASSERT(rp->r_flags & RG_LOCK);

	/*	If the region is still in use, then don't free it.
	 */

	if(rp->r_refcnt != 0)
		return;


	/*
	 * Decrement use count on associated inode
	 */

	if (ip = rp->r_iptr) {
		if (rp->r_type == RT_STEXT)
			ip->i_flag &= ~ITEXT;
		if (ip->i_flag&ILOCK)
			ip->i_count--;
		else
			iput(ip);
	}

	/*	Free the memory pages and the page tables and
	 *	disk use maps.  These latter are always allocated
	 *	together in pairs in a contiguous 256 word piece
	 *	of kernel virtual address space.  Note that the
	 *	pfree for the first page table is special because
	 *	pages may not have been allocated from the beginning
	 *	of the segment.  The last page table is also special
	 *	since it may not have been fully allocated.
	 */
	
	tsize = rp->r_pgoff + rp->r_pgsz;
	i = ptots(rp->r_pgoff);
	lim = ptos(tsize);
	ASSERT(i <= lim);

	for(  ;  i < lim  ;  i++){
		pt = rp->r_list[i];
		ASSERT(pt != 0);
		size = tsize - stopg(i);
		if(size > NPGPT)
			size = NPGPT;
		if(rp->r_pgoff > stopg(i)){
			size -= rp->r_pgoff - stopg(i);
			pt += rp->r_pgoff - stopg(i);
		}
		pfree(rp, pt, pt + NPGPT, size);
		sptfree(rp->r_list[i], 2, 1);
	}

	/*	Free the list.
	 */
	
	sptfree(rp->r_list, rp->r_listsz, 1);

	if(rp->r_type != RT_PRIVATE) {
		/*
		 * Remove from active list
		 */

		rp->r_back->r_forw = rp->r_forw;
		rp->r_forw->r_back = rp->r_back;
	}

	regrele(rp);
	*rp = nullregion;

	/*
	 * Link into free list
	 */

	rp->r_forw = rfree.r_forw;
	rp->r_back = &rfree;
	rfree.r_forw->r_back = rp;
	rfree.r_forw = rp;
}

/*
 * Attach a region to a process' address space
 */
preg_t *
attachreg(rp, up, vaddr, type, prot)
register reg_t *rp;	/* pointer to region to be attached */
struct user *up;	/* pointer to U-block of process (needed by fork) */
caddr_t	vaddr;		/* virtual address to attach at */
int	type;		/* Type to make the pregion. */
int	prot;		/* permissions for segment table entries. */
{
	register preg_t *prp;
	register int *st, ste, i;

	ASSERT(rp->r_flags & RG_LOCK);

	/*	Check attach address.
	 *	It must be segment aligned.
	 */

	if((int)vaddr & SOFFMASK) {
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

	prp->p_reg = rp;
	prp->p_regva = vaddr;
	prp->p_type = type;
	if(prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;

	/*	Check that region does not go beyond end of virtual
	 *	address space.
	 */

	if (chkattach(up, prp, ((uint)vaddr < P0SPACE))) {

		/* Region beyond end of address space.
		 * Undo what has been done so far
		 */

		*prp = nullpregion;
		u.u_error = EINVAL;
		return(NULL);
	}

	/*	Load the segment table.
	 */

	loadstbl(up, prp, 0, ((unsigned)prp->p_regva < P0SPACE) );

	ASSERT(up->u_procp->p_flag&SLOAD);
	++rp->r_incore;
	++rp->r_refcnt;
	up->u_procp->p_size += rp->r_pgsz;

	return(prp);
}

/*
 * Detach a region from the current process' address space
 */
void
detachreg(up, prp)
struct user *up;
register preg_t *prp;
{
	register reg_t	*rp;
	register int	i;
	register int	*st;
	struct proc *pp;
	spte_t *p1br;
	register int segnum, pnum, lotohi, sp;

	/* is this p0 or p1 space? */
	lotohi = (unsigned)prp->p_regva < P0SPACE;
	pp = up->u_procp;
	rp = prp->p_reg;
	ASSERT(rp != NULL);
	ASSERT(rp->r_flags & RG_LOCK);
	if (rp->r_list == NULL)
		goto nullreg;
	ASSERT(rp->r_list[0] != 0);

	/*
	 * Invalidate segment table entries pointing at the region
	 */
	if (lotohi) {
		segnum = snum(prp->p_regva);
		pnum = svtop(pp->p_spt);
		sp = pnum + segnum;
		st = &sbrpte[sp];
	} else {
		if (curproc == pp)
			p1br = (spte_t *)mfpr(P1BR);
		else
			p1br = (spte_t *)up->u_pcb.pcb_p1br;
		sp = svtop(p1br + svtop(prp->p_regva));
		st = &sbrpte[sp]; 
	}

	for (i = rp->r_pgsz + rp->r_pgoff; i > 0; i -= NPGPT) {
		*st++ = 0;
		invsatb(SYSATB, ptosv(sp++));
	}

	/*	Decrement process size by size of region.
	 */

	up->u_procp->p_size -= rp->r_pgsz;

	/*
	 * Decrement use count and free region if zero
	 * and RG_NOFREE is not set, otherwise unlock.
	 */
nullreg:
	--rp->r_incore;
	if (--rp->r_refcnt == 0 && !(rp->r_flags & RG_NOFREE))
		freereg(rp);
	else
		regrele(rp);
	
	/*	Clear the proc region we just detached.
	 */
	
	for (i = prp - up->u_procp->p_region; i < pregpp-1; i++, prp++)
		*prp = *(prp+1);

	*prp = nullpregion;
}

/*
 * Duplicate a region
 *	rp: region to be dup'd
 */

reg_t *
dupreg(rp, type)
register reg_t *rp;
int type;
{
	register int	i;
	register int	j;
	register int	size;
	register pte_t	*ppte;
	register pte_t	*cpte;
	register pte_t	**plp;
	register pte_t	**clp;
	register reg_t	*rp2;
	int protection;

	ASSERT(rp->r_flags & RG_LOCK);

	/* If region is shared, there is no work to do.
	 * Just return the passed region.  The region reference
	 * counts are incremented by attachreg
	 */

	if((rp->r_type != RT_PRIVATE) && (type == SEG_CW))
		return(rp);

	/*
	 * Need to copy the region.
	 * Allocate a region descriptor
	 */

	if ((rp2 = allocreg(rp->r_iptr, rp->r_type)) == NULL)
		return(NULL);
	
	/*	Allocate a list for the new region.
	 */

	rp2->r_listsz = rp->r_listsz;
	rp2->r_list = (pte_t **)sptalloc(rp2->r_listsz, PG_V | PG_KW, 0);
	if(rp2->r_list == NULL){
		freereg(rp2);
		return(NULL);
	}


	/*
	 * Copy pertinent data to new region
	 */

	rp2->r_pgsz = rp->r_pgsz;
	rp2->r_pgoff = rp->r_pgoff;
	rp2->r_nvalid = rp->r_nvalid;
	rp2->r_filesz = rp->r_filesz;

	/* Scan the parents page table  list and fix up each page table.
	 * Allocate a page table and map table for the child and
	 * copy it from the parent.
	 */

	if (type != SEG_CW)
		protection = (type == SEG_RO) ? PTE_UR : PTE_UW;
	for(i = 0  ;  i < ptos(rp->r_pgoff + rp->r_pgsz)  ;  i++){
		plp = &rp->r_list[i];
		clp = &rp2->r_list[i];

		/* Allocate a page table and map table for the child.
		 */

		if((cpte = (pte_t *)sptalloc(2, PG_V|PG_KW, 0)) == 0){

			/* We could not allocate a page table and map.
			 * Set the size to the number of page tables
			 * we have actually allocated and then free
			 * the region.
			 */

			rp2->r_pgsz = stopg(i);
			freereg(rp2);
			return(NULL);
		}

		/* If we are before the offset, then there are no
		 * pages to duplicate.  Otherwise, compute the number
		 * of pages.  The first page table may be special
		 * because of the offset and the last because the
		 * size may not be an integral number of segments.
		 */

		*clp = cpte;
		if(i < ptots(rp->r_pgoff))
			continue;
		ppte = *plp;

		/* Get the total number of unmapped pages remaining.
		 * This is the total size of the region minus the
		 * number of segments for which we have allocated
		 * page tables already.
		 */

		size = rp->r_pgoff + rp->r_pgsz - stopg(i);

		/* If this size is greater than a segment, then
		 * we will only process a segment.
		 */

		if(size > NPGPT)
			size = NPGPT;

		/* Check for the first segment after the offset.
		 * This is not a full segment in general.
		 */

		if(rp->r_pgoff > stopg(i)){
			size -= rp->r_pgoff - stopg(i);
			ppte += rp->r_pgoff - stopg(i);
			cpte += rp->r_pgoff - stopg(i);
		}

		ASSERT(size > 0  &&  size <= NPGPT);

		/* Check each parents page and then copy it to
		 * the childs pte.  Also check the map table
		 * entries.
		 */

		for(j = 0  ;  j < size  ;  j++, ppte++, cpte++){
			dbd_t		map;

			/*
			 *	if the pte is invalid, but the page
			 *	still belongs to this region, set the
			 *	valid bit. speeds up the tests for valid
			 *	pages which follow
			 */
			if (!ppte->pgm.pg_v)
				if (ppte->pgm.pg_ref || ppte->pgm.pg_ndref) {
					ppte->pgm.pg_v = 1;
					ppte->pgm.pg_ref = 1;
					ppte->pgm.pg_ndref = 1;
				}

			/* Change writeable pages to copy-on write.
			 */

			if (type == SEG_CW) {
				if(ppte->pgm.pg_prot == PTE_UW) {
					ppte->pgm.pg_prot = PTE_UR;
					ppte->pgm.pg_cw = 1;
				}

				*cpte = *ppte;
			} else {
				*cpte = *ppte;
				cpte->pgm.pg_prot = protection;
			}

			/* if the page is in core, increment
			 * the use count on the pfdat */

			if (ppte->pgm.pg_v) {
				struct pfdat *pfd;

				pfd = pfdat + ppte->pgm.pg_pfn;
				ASSERT(pfd->pf_use != 0); 
				pfd->pf_use++;
			}

			/* Increment the swap use count for pages which
			 * are on swap.
			 */

			map = *(dbd_t *)(ppte + NPGPT);
			if (map.dbd_type == DBD_SWAP) {
				ASSERT(swpuse(&map) != 0);
				if(!swpinc(&map, "dupreg")){

					/* swap use count overflowed.
					 * Free the region and return
					 * an error.
					 */

					((dbd_t *)(cpte + NPGPT))->dbd_type =
						DBD_NONE;
					freereg(rp2);
					u.u_error = ENOMEM;
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

growreg(up, prp, change, fill, type)
struct user *up;
register preg_t *prp;
{
	register pte_t	*pt;
	register int 	i;
	register int	j;
	register reg_t	*rp;
	register int	size;
	register int	osize;
	int lotohi;
	int start, end;
	int k;
	struct dbd *dbd;

	rp = prp->p_reg;

	ASSERT(rp != NULL);
	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(change >= 0 || (-change <= rp->r_pgsz));
	ASSERT(rp->r_refcnt == 1);

	if (change == 0)
		return(0);

	/* is this p0 or p1 space? */
	lotohi = (unsigned)prp->p_regva < P0SPACE;

	osize = rp->r_pgoff + rp->r_pgsz;

	if (change < 0) {

		/*	The region is being shrunk.  Compute the new
		 *	size and free up the unneeded space.
		 */

		if (lotohi) {
			start = osize + change;
			end = osize;
		} else {
			start = rp->r_pgoff;
			end = rp->r_pgoff - change;
		}

		i = ptots(start);

		for(  ;  i < ptos(end)  ;  i++){
			/*	Free up the allocated pages for
			 *	this segment.
			 */

			pt = rp->r_list[i];
			size = end - stopg(i);
			if(size > NPGPT)
				size = NPGPT;
			if(start > stopg(i)){
				size -= start - stopg(i);
				pt += start - stopg(i);
			}
			pfree(rp, pt, pt + NPGPT, size);
		}

		/*	Free up the page tables which we no
		 *	longer need.
		 */

		rptexpand(prp, change, lotohi);
	} else {
		/*	We are expanding the region.  Make sure that
		 *	the new size is legal and then allocate new
		 *	page tables if necessary.
		 */

		if (chkgrowth(up, prp, change, lotohi)) {
			up->u_error = ENOMEM;
			return(-1);
		}
		if (rptexpand(prp,change,lotohi)) {
			up->u_error = ENOMEM;
			return(-1);
		}

		/*	Initialize the new page tables and allocate
		 *	pages if required.
		 */

		if (lotohi) {
			start = osize;
			end = osize + change;
		} else {
			start = rp->r_pgoff;
			end = rp->r_pgoff + change;
		}

		i = ptots(start);
		for( ; i < ptos(end) ; i++){
			pt = rp->r_list[i];
			size = end - stopg(i);
			if(size > NPGPT)
				size = NPGPT;
			if(start > stopg(i)){
				size -= start - stopg(i);
				pt += start - stopg(i);
			}

			ASSERT(size > 0  &&  size <= NPGPT);

			if (lotohi) {
				if(fill){
					register int	fillsz;

					fillsz = min(fill, size);
					if(ptmemall(rp, pt, fillsz, 1))
					panic("growreg - ptmemall failed");
					dbd = (dbd_t *)(pt + NPGPT);
					for (k=0; k<fillsz; k++, dbd++)
						dbd->dbd_type = type;
					if (type == DBD_DZERO) {
						pte_t *tmp;

						tmp = pt;
						for (k=0; k<fillsz; k++) {
						   clearpage(tmp->pgm.pg_pfn);
						   tmp++;
						}
					}

					fill -= fillsz;
					size -= fillsz;
					pt += fillsz;
				}

				if(size){
					register	proto;
					pte_t		pte;

					pte.pgi.pg_pte = 0;
					pte.pgm.pg_prot = (type==DBD_NONE)?
								0 : PTE_UW;

					proto = pte.pgi.pg_pte;

					while (--size >= 0){
					   pt->pgi.pg_pte = proto;
					   ((dbd_t *)(pt+NPGPT))->dbd_type =
									type;
					   pt++;
					}
				}
			} else {
				int demsize;

				if (fill < change) {
					register	proto;
					pte_t		pte;

					pte.pgi.pg_pte = 0;
					pte.pgm.pg_prot = (type==DBD_NONE)?
								0 : PTE_UW;

					proto = pte.pgi.pg_pte;

					demsize = min(size, change-fill);
					fill += demsize;
					size -= demsize;
					while (demsize-- > 0) {
					   pt->pgi.pg_pte = proto;
					   ((dbd_t *)(pt+NPGPT))->dbd_type =
									type;
					   pt++;
					}
				}

				if (size) {
					if (ptmemall(rp, pt, size, 1))
					panic("growreg p1 - ptmemall failed");
					dbd = (dbd_t *)(pt + NPGPT);
					for (k=0; k<size; k++, dbd++)
						dbd->dbd_type = type;
					if (type == DBD_DZERO) {
						pte_t *tmp;

						tmp = pt;
						for (k=0; k<size; k++) {
						   clearpage(tmp->pgm.pg_pfn);
						   tmp++;
						}
					}

					pt += size;
				}
			}
		}
	}

	loadstbl(up, prp, change, lotohi);

	rp->r_pgsz += change;
	up->u_procp->p_size += change;
	if (prp->p_type == PT_STACK)
		up->u_ssize += change;
	return(1);
}

/*
 * Check that grow of a pregion is legal
 */
chkgrowth(up, prp, change, lotohi)
register struct user *up;
register preg_t *prp;
register int change;
{
	register reg_t *rp;
	register int	*st, size, n, tos;
	struct proc *pp;
	spte_t *p0br, *p1br;
	int p1lr, p0lr;
	int gap, extent;

	pp = up->u_procp;
	rp = prp->p_reg;
	size = rp->r_pgoff + rp->r_pgsz;

	gap = (pp->p_nspt * NPGPT) - pp->p_p0size - pp->p_p1size;

	if (lotohi) {
		extent = svtop(prp->p_regva) + size + change;
		if (extent > pp->p_p0size + gap) {
			if (ptexpand(up, ptos(extent-(pp->p_p0size+gap))) == -1){
				return(-1);
			}
		}

		if (extent > pp->p_p0size) {
			pp->p_p0size = extent;
			up->u_pcb.pcb_p0lr = extent | 0x04000000;
			if (pp == curproc)
				mtpr(P0LR, extent);
		}

		n = ((size%NPGPT) == 0) ? 0 : NPGPT - (size % NPGPT);
		if (change <= n)
			return(0);
		change -= n;
		st = &sbrpte[svtop(pp->p_spt + svtop(prp->p_regva))
						+ ptos(size)];
	} else {
		/*	Set n to the number of pages left in the
		 *	last segment of the region
		 */
		n = rp->r_pgoff;
		if(n == NPGPT)
			n = 0;
		/*
		 *	If change <= n the growth is legal, otherwise
		 *	check the segment table for conflicts
		 */
		if(change <= n)
			return(0);
		change -= n;

		/*
		 *	for stack growths, the change must be an integral
		 *	number of segments
		 */
		change = (change + NPGPT - 1) & ~(NPGPT - 1);

		if ((unsigned)prp->p_regva >= (unsigned)0x80000000)
			extent = change;
		else
			extent = (REGSZPGS - svtop(prp->p_regva)) + change;

		if (extent > pp->p_p1size) {
			if (ptexpand(up, ptos(change)) == -1){
				return(-1);
			}
		}

		if (extent > pp->p_p1size) {
			pp->p_p1size = extent;
			up->u_pcb.pcb_p1lr = (REGSZPGS - extent);
			if (pp == curproc)
				mtpr(P1LR, REGSZPGS - extent);
		}

		if (pp == curproc) {
			p1br = (spte_t *)mfpr(P1BR);
		} else {
			p1br = (spte_t *)up->u_pcb.pcb_p1br;
		}
		st = &sbrpte[svtop(p1br +
				svtop(prp->p_regva - ptob(change)) )];
	}
	for(; change > 0; change -= NPGPT)
		if(*st++ & PG_V){
			return(-1);
		}
	return(0);
}

/*
 * Check that attach of a pregion is legal
 */
chkattach(up, prp, lotohi)
register struct user *up;
register preg_t *prp;
{
	register reg_t *rp;
	register int	*st, size, n, tos;
	struct proc *pp;
	spte_t *p0br, *p1br;
	int p1lr, p0lr;
	int gap, extent;

	pp = up->u_procp;
	rp = prp->p_reg;
	size = rp->r_pgoff + rp->r_pgsz;

	gap = (pp->p_nspt * NPGPT) - pp->p_p0size - pp->p_p1size;

	if (lotohi) {
		extent = svtop(prp->p_regva) + size;
		if (extent > pp->p_p0size + gap) {
			if (ptexpand(up, ptos(extent-pp->p_p0size-gap)) == -1){
				return(-1);
			}
		}

		if (extent > pp->p_p0size) {
			pp->p_p0size = extent;
			up->u_pcb.pcb_p0lr = extent | 0x04000000;
			if (pp == curproc)
				mtpr(P0LR, extent);
		}

		st = &sbrpte[svtop(pp->p_spt + svtop(prp->p_regva))];
	} else {
		if ((uint)prp->p_regva >= (uint)0x80000000)
			extent = 0;
		else
			extent = (REGSZPGS - svtop(prp->p_regva));

		if (extent > pp->p_p1size + gap) {
			if (ptexpand(up, ptos(extent-pp->p_p1size-gap)) == -1){
				return(-1);
			}
		}

		if (extent > pp->p_p1size) {
			pp->p_p1size = extent;
			up->u_pcb.pcb_p1lr = (REGSZPGS - extent);
			if (pp == curproc)
				mtpr(P1LR, REGSZPGS - extent);
		}

		if (pp == curproc) {
			p1br = (spte_t *)mfpr(P1BR);
		} else {
			p1br = (spte_t *)up->u_pcb.pcb_p1br;
		}
		st = &sbrpte[svtop(p1br + svtop(prp->p_regva) )];
	}
	for(; size > 0; size -= NPGPT)
		if(*st++ & PG_V){
			return(-1);
		}
	return(0);
}

loadstbl(up, prp, change, lotohi)
struct user	*up;
preg_t		*prp;
register int	change;
{
	register reg_t	*rp;
	register int	*st;
	register struct proc *p;
	register int	pnum, segnum, staddr, enduser;
	register int	size;
	spte_t *p1br;

	rp = prp->p_reg;
	p = up->u_procp;
	if (curproc == p)
		p1br = (spte_t *)mfpr(P1BR);
	else
		p1br = (spte_t *)up->u_pcb.pcb_p1br;

	size = rp->r_pgoff + rp->r_pgsz;
	if (change < 0) {
		register int	*stlim;

		change = -change;
		if (lotohi) {	
			st = &sbrpte[svtop(p->p_spt)] + snum(prp->p_regva) +
				ptos(size - change);
			stlim = &sbrpte[svtop(p->p_spt)] + snum(prp->p_regva) +
				ptos(size);
		} else {
			st = &sbrpte[svtop(p1br + svtop(prp->p_regva
					- ptob(change)))];
			stlim = (int *)&sbrpte[svtop(p1br + svtop(prp->p_regva))];
		}
		for(  ;  st < stlim ;  st++)
			*st = 0;
	} else {
		register pte_t	**lp;
		register pte_t	**lplim;
		register int	physaddr;
		register int	prot;
		int pnum;

		prot = PG_V | PG_KW;

		if (lotohi) {
			pnum = svtop(p->p_spt);
			st = &sbrpte[pnum + snum(prp->p_regva)];
		} else {
			st = &sbrpte[svtop(p1br + svtop(prp->p_regva))];
		}

		lp = rp->r_list;
		lplim = &lp[ptos(rp->r_pgoff + rp->r_pgsz + change)];

		for(  ;  lp < lplim  ;  lp++){
			physaddr = ((spte_t *)svtopte(*lp))->pgm.pg_pfn;
			*st++ = mkpte(prot, physaddr);
		}
	}
	return(1);
}

/*
 * Expand page table space for process
 *	change	-> # of page tables to add
 */

/*ARGSUSED*/
ptexpand(up, change)
struct user *up;
int change;
{
	register spte_t *nspt, *ospt;
	register pt_t *va;
	register int i;
	int nmap, nsize;
	int ssize;
	struct proc *p;
	int myself;

	p = up->u_procp;
	myself = (curproc == p);

	ssize = REGSZPGS - (myself ? mfpr(P1LR) : up->u_pcb.pcb_p1lr);
	if (change <= 0)  {
		goto out;
	}

	if ((nsize = p->p_ptsize + change) > maxupts) {
		goto out;
	}

	if ((nmap = malloc(sptmap, nsize)) == 0) {
		/*
		 * no slots in system pt
		 */
		printf("No kernel virtual space\n");
		printf("ptexpand: malloc failed. nsize= %x\n",nsize);
out:
		up->u_error = ENOMEM;
		return(-1);
	}

	/*
	 * copy spt entries for upt pages describing stack
	 */
	ospt = (spte_t *)(sbrpte + p->p_ptaddr+p->p_ptsize);
	nspt = (spte_t *)(sbrpte + nmap+nsize);
	va = (pt_t *) vtop(nmap+nsize, 0);

	/*
	 * copy through current stack size
	 */
	for (i = (ssize + NPGPT - 1)/NPGPT; --i >= 0; ) {
		*--nspt = *--ospt;
		invsatb(SYSATB, --va);
	}

	/*
	 * clear and insert new pages
	 */
	for (i = change; --i >= 0; ) {
		--nspt;
		nspt->pgi.pg_pte = 0;
		invsatb(SYSATB, --va);
	}

	/*
	 * split the fragmented page
	 */
	if ( (-ssize)&(NPGPT-1) != 0) {
		sptmemall(nspt, 1);
		bzero(va, sizeof(pt_t));
		{
			register pte_t *npte;
			register pte_t *opte;
	
			npte = (pte_t *) va;
			opte = (pte_t *) baseseg[ospt - (spte_t *)sbrpte];
			for (i = (-ssize)&(NPGPT - 1); --i >= 0; ) {
				*npte++ = *opte;
				opte++->pgi.pg_pte = 0;
			}
		}
	}

	/*
	 * copy spt entries for upt pages describing text, data
	 */
	for (i = p->p_ptsize - ((ssize + NPGPT - 1)/NPGPT); --i >= 0; ) {
		*--nspt =  *--ospt;
		invsatb(SYSATB, --va);
	}

	ASSERT (nspt - (spte_t *)sbrpte == nmap) ;

	/*
	 * the big switch from one section of spt to another
	 */
	{
		register oldspl;

		oldspl = spl6();	/* lock out clock() */
		i = p->p_ptaddr;
		p->p_ptaddr = nmap;
		p->p_ptsize = nsize;
		p->p_spt = (int *) (vtop(p->p_ptaddr, 0));
		p->p_nspt = p->p_ptsize;
	
		up->u_pcb.pcb_p0br = (int) vtop(nmap, 0);
		if (myself) mtpr(P0BR, up->u_pcb.pcb_p0br);
		{
			register spte_t *vadr;
	
			vadr =  vtop(nmap+nsize, 0) - REGSZPGS;
			up->u_pcb.pcb_p1br = (int) vadr;
			if (myself) mtpr(P1BR, vadr);
		}
	
		clratb(USRATB);
		splx(oldspl);
	}
	/*
	 * emancipate former slots
	 */
	{
		int j;

		for (j=0; j<nsize-change; j++) {
			sbrpte[i+j] = 0;
			invsatb(SYSATB, ptosv(i+j) );
		}
	}
	mfree(sptmap, nsize - change, i);

	return(change);
}


/*
 * Expand user page tables for a region 
 */
rptexpand(prp, change, lotohi)
register preg_t *prp;
{
	register reg_t *rp;
	register pte_t	**lp;
	register int	osize;
	register int	nsize;
	register pte_t	**lp1, **lp2;
	register int	i;

	/* Calculate the new size in pages.
	 */

	rp = prp->p_reg;

	if (lotohi)
		osize = rp->r_pgoff + rp->r_pgsz;
	else
		osize = rp->r_pgsz;
	nsize = osize + change;

	/*	If we are shrinking the region, then free up
	 *	the page tables and map tables.  Use a smaller
	 *	list if possible.
	 */

	if(ptos(nsize) < ptos(osize)){
		if (lotohi) {
			lp = &rp->r_list[ptos(nsize)];
			for(  ;  lp < &rp->r_list[ptos(osize)]  ;  lp++){
				sptfree(*lp, 2, 1);
				*lp = 0;
			}
		} else {
			int diff;

			diff = ptos(osize) - ptos(nsize);
			lp1 = rp->r_list;
			lp2 = &rp->r_list[diff];
			while (lp2 < &rp->r_list[ptos(osize)]) {
				sptfree(*lp1, 2, 1);
				*lp1++ = *lp2;
				*lp2++ = 0;
			}
			while (*lp1 != 0) {
				sptfree(*lp1, 2, 1);
				*lp1++ = 0;
			}
			rp->r_pgoff -= stopg(diff);
			prp->p_regva += stob(diff);
		}
		if(ptos(ptos(nsize)) < rp->r_listsz){
			if(nsize > 0){
				lp2 = (pte_t **)sptalloc(ptos(ptos(nsize)), PG_V | PG_KW, 0);
				if(lp2 == 0)
					return(0);
				bcopy(rp->r_list, lp2, ptos(nsize)*NBPW);
			}else{
				lp2 = 0;
			}
			sptfree(rp->r_list, rp->r_listsz, 1);
			rp->r_list = lp2;
			rp->r_listsz = ptos(ptos(nsize));
		}
	}
		
	/*	If the region shrunk or didn't grow by enough to
	 *	require any more page tables, then we are done.
	 */


	if(change <= 0){
		if (!lotohi)
			rp->r_pgoff -= change;
		return(0);
	}
	
	/*	If the region grew into the next segment,  then we 
	 *	must allocate one or more new page and map tables.
	 *	See if we have enough space in the list for the
	 *	new segments.  If not, allocate a new list and
	 *	copy over the old data.
	 */
	i = ptos(ptos(nsize));
	if(i > rp->r_listsz){
		if((lp2 = (pte_t **)sptalloc(i, PG_V | PG_KW, 0)) == 0){
			return(-1);
		}
			
		if(rp->r_list){
			bcopy(rp->r_list, lp2, ptos(osize)*NBPW);
			sptfree(rp->r_list, rp->r_listsz, 1);
		}
		rp->r_list = lp2;
		rp->r_listsz = i;
	}

	/*
	 * Allocate a new set of page tables and disk maps.
	 */

	if (lotohi) {
		lp1 = lp = &rp->r_list[ptos(osize)];
		lp2 = &rp->r_list[ptos(nsize)];
	} else {
		int diff;

		diff = ptos(nsize) - ptos(osize);
		lp1 = &rp->r_list[ptos(osize)];
		lp2 = &rp->r_list[ptos(nsize)];
		while (lp1 > rp->r_list)
			*(--lp2) = *(--lp1);
		lp = lp1;
		rp->r_pgoff += (stopg(diff) - change);
		prp->p_regva -= stob(diff);
	}

	for(  ;  lp < lp2  ;  lp++){
		*lp = (pte_t *)sptalloc(2, PG_V | PG_KW, 0);
		if(*lp == 0){
			while(--lp >= lp1)
				sptfree(*lp, 2, 1);
			return(-1);
		}
	}

	return(0);
}

loadreg(prp, vaddr, ip, off, count)
register preg_t		*prp;
caddr_t			vaddr;
register struct inode	*ip;
{
	register reg_t	*rp;
	register int	change;

	/*	Grow the region to the proper size to load the file.
	 */
	
	ASSERT((int)vaddr < 0x40000000);

	rp = prp->p_reg;
	change = vaddr - prp->p_regva;

	if(growreg(&u, prp, btop(change + count), 0, DBD_DZERO) < 0) {
		return(-1);
	}

	rp->r_pgoff = btotp(change);
	rp->r_pgsz -= btotp(change);

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
	readi(ip);

	reglock(rp);
	rp->r_flags |= RG_DONE;
	if(rp->r_flags & RG_WAITING){
		rp->r_flags &= ~RG_WAITING;
		wakeup(&rp->r_flags);
	}
	if(u.u_count) {
		return(-1);
	}

	/*	Clear the last (unused)  part of the last page.
	 */
	
	vaddr += count;
	count = ptob(1) - poff(vaddr);
	if(count > 0 && count < ptob(1))
		if(uclear(vaddr, count) < 0) {
			return(-1);
		}

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
	register int	nppblk;
	register reg_t	*rp;
	int		edev;
	int		gap;
	int		seglim;
	int		blkno;
	int		boff;
	int		size;
	dbd_t		*dbd;
	short		firsttime;
	int		page_in_blk;

	/*	If the block number list is not built,
	 *	then build it now.
	 */
	
	if(ip->i_map == 0)
		bldblklst(ip);

	/*	Get region pointer and effective device number.
	 */

	rp = prp->p_reg;
	edev = ip->i_dev;

	/*	Compute the number of file system blocks in a page.
	 *	This depends on the file system block size.
	 */

	nppblk = FsBSIZE(edev) / NBPP;

	/*	Allocate invalid pages for the gap at the start of
	 *	the region and demand-fill pages for the actual
	 *	text.
	 */
	
	gap = vaddr - prp->p_regva;
	if(growreg(&u, prp, btotp(gap), 0, DBD_NONE) < 0) {
		return(-1);
	}
	if(growreg(&u, prp, btop(count+gap) - btotp(gap), 0, DBD_DFILL) < 0)
		return(-1);
	
	rp->r_pgoff = btotp(gap);
	size = rp->r_pgsz;
	rp->r_pgsz -= btotp(gap);
	rp->r_filesz = count + off;
	
	gap = btotp(gap);  /* Gap in pages. */
	boff = FsBNO(edev, off);	/* File offset in blocks. */
	i = ptots(gap);
	seglim = ptos(size);

	page_in_blk = (FsBOFF(edev, off)) / NBPP;
	firsttime = 1;

	for(  ;  i < seglim  ;  i++){
		register int	lim;
		register pte_t	*pt;

		if(gap > stopg(i))
			j = gap - stopg(i);
		else
			j = 0;
		ASSERT(j >= 0  &&  j < NPGPT);

		lim = size - stopg(i);
		if(lim > NPGPT)
			lim = NPGPT;
		ASSERT(lim >= j  &&  lim <= NPGPT);

		pt = (pte_t *)rp->r_list[i] + j;
		dbd = (dbd_t *)pt + NPGPT;

		for(  ;  j < lim ;  j++, pt++, dbd++) {

		       /*	If these are private pages, then make
		 	*	them copy-on-write since they will
		 	*	be put in the hash table.
		 	*/
			if(rp->r_type == RT_PRIVATE){
				pt->pgm.pg_prot = PTE_UR;
				pt->pgm.pg_cw = 1;
			}
			dbd->dbd_blkno = ip->i_map[boff];
			dbd->dbd_type = DBD_FILE;

			if (firsttime) {
				dbd->dbd_adjunct = DBD_FIRST;
				firsttime = 0;
			} else
				dbd->dbd_adjunct = 0;

			if (page_in_blk == 0) {
				dbd->dbd_adjunct |= DBD_FP;
			}

			page_in_blk++;
			if (page_in_blk >= nppblk) {
				page_in_blk = 0;
				boff++;
			}
		}
	}

	/*	Mark the last page for special handling
	 */
	
	dbd[-1].dbd_adjunct |= DBD_LAST;

	rp->r_flags |= RG_DONE;
	return(0);

}
/*	Find the region corresponding to a virtual address.
 */

preg_t *findreg(p, vaddr)
register struct proc	*p;
register caddr_t	vaddr;
{
	register preg_t	*prp;
	register preg_t	*oprp;

	oprp = p->p_region;
	for(prp = &p->p_region[1] ; prp->p_reg ; prp++)
		if(vaddr >= prp->p_regva  &&
		   (prp->p_regva > oprp->p_regva | vaddr < oprp->p_regva))
			oprp = prp;
	if(vaddr >= oprp->p_regva)
		return(oprp);
	panic("findreg - no match");
	/*NOTREACHED*/
}

/*	Find the pregion of a particular type.
 */

preg_t *
findpreg(pp, type)
register struct proc *pp;
register int	type;
{
	register preg_t	*prp;

	for(prp = pp->p_region ; prp->p_reg ; prp++){
		if(prp->p_type == type)
			return(prp);
	}

	/*	We stopped on an unused region.  If this is what
	 *	was called for, then return it unless it is the
	 *	last region for the process.  We leave the last
	 *	region unused as a marker.
	 */

	if((type == PT_UNUSED)  &&  (prp < &pp->p_region[pregpp - 1]))
		return(prp);
	return(NULL);
}

/*
 * Change protection of ptes for a region
 */
void
chgprot(prp, prot)
register preg_t	*prp;
{
	register reg_t	*rp;
	register pte_t	*pt;
	register int	i;
	register int	j;
	register int	seglim;
	register int	pglim;

	if(prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;
	else
		prp->p_flags &= ~PF_RDONLY;
	rp = prp->p_reg;

	/*	Look at all of the segments of the region.
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
		pt = rp->r_list[i] + j;
		pglim = rp->r_pgoff + rp->r_pgsz - stopg(i);
		if(pglim > NPGPT)
			pglim = NPGPT;
		
		ASSERT(j >= 0  &&  j <= NPGPT);
		ASSERT(pglim >= j  &&  pglim <= NPGPT);

		for(  ;  j < pglim  ;  j++, pt++){

			/*	set protection
			 */

			pt->pgm.pg_prot = (prot==SEG_RO) ? PTE_UR : PTE_UW;
		}
	}
	clratb(USRATB);
}
