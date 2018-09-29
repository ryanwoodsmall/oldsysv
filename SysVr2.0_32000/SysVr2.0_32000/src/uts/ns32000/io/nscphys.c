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
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/sysinfo.h"

/*
 * Check alignment for physical I/O
 */

aln_chk()
{
	if (((int)u.u_base & 0x1ff) || (u.u_count & 0x1ff)) {
		return(1);
	}
	return (0);
}

/*
 * Get physical block numbers for physio
 *   must be called with disk/tape ints masked
 */

physmap(bp, nbl, bufp)
register struct buf *bp;
int nbl;
int *bufp;
{
	register int i, *pt;

	pt = (int *)bp->b_pt;
	if ((int)bp->b_un.b_addr & 0x200)
		pt++;
	/*
	 * if bp->b_proc, the page tables belong to a process
	 *   and must be mapped at this time.
	 */
	if (bp->b_proc)
		u3load(bp->b_proc->p_addr);
	for (i=0; i<nbl; i++)
		bufp[i] = *pt++ & PFNMASK;
}

/*
 * Fake out physical I/O since request is not aligned
 *  or not even multiple of BSIZE
 *  note - size is 512 or 1024 (buffers better be at least 1024)
 */

fak_phys(strat, dev, rw, size)
int (*strat)();
{
	register struct buf *bp;
	register n;

	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;
	bp = geteblk();
	bp->b_bcount = size;
	syswait.physio++;

	do {
		bp->b_dev = dev;
		bp->b_blkno = u.u_offset >> 9;
		bp->b_flags = B_BUSY | B_AGE | rw;
		n = u.u_count > size ? size : u.u_count;
		if (rw == B_WRITE) {
			if (n < size)
				clrbuf(bp);
			iomove(bp->b_un.b_addr, n, B_WRITE);
			if (u.u_error)
				break;
		}

		(*strat)(bp);
		iowait(bp);

		if (rw == B_READ && u.u_error == 0)
			iomove(bp->b_un.b_addr, n, B_READ);
	} while (u.u_error==0 && u.u_count!=0);

	bp->b_flags |= B_STALE | B_AGE;
	brelse(bp);
	syswait.physio--;
}
