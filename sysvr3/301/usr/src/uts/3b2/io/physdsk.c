/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/physdsk.c	10.5"
/* 
 *		3B2 Computer UNIX Integral Disk Driver Common
 *		DMA break-up routine (used by integral hard and 
 *		floppy disk drivers)
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/region.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/inline.h"

/*
 *	Break up the request that came from physio into chunks of
 *	contiguous memory so we can get around the DMAC limitations
 *	We must be sure to pass at least 512 bytes (one sector) at a
 *	time (except for the last request).   */

/* determine no. bytes till page boundary */

#define	pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))
#define dbbnd(a)	(NBPSCTR - ((NBPSCTR - 1) & (int)(a)))

dma_breakup(strat, bp)
int (*strat)();
register struct buf *bp;
{
	register char *va;
	register int cc, rw;
	char *kseg();

	rw = bp->b_flags & B_READ;

	if (dbbnd(u.u_base) <  NBPSCTR) {

		/* user area is not aligned on block (512 byte) boundary 
		 * so copy data to a contiguous kernel buffer and do the
		 * dma from there
		 */

		va = kseg(1);
		if (va == NULL) {
			bp->b_flags |= B_ERROR | B_DONE;
			bp->b_error = EAGAIN;
			return;
		}
		bp->b_un.b_addr = va;
		u.u_segflg = 0;
		do {
			bp->b_blkno = btod(u.u_offset);
			bp->b_bcount = cc = min(u.u_count, NBPP);
			bp->b_flags &= ~B_DONE;

			if (rw == B_READ) {
				(*strat)(bp);
				spl6();
				while ((bp->b_flags & B_DONE) == 0) {
					bp->b_flags |= B_WANTED;
					sleep(bp, PRIBIO);
				}
				spl0();
				if (bp->b_flags & B_ERROR) {
					unkseg(va);
					return;
				}
				iomove(va, cc, rw);
			} else {
				iomove(va, cc, rw);
				(*strat)(bp);

				spl6();
				while ((bp->b_flags & B_DONE) == 0) {
					bp->b_flags |= B_WANTED;
					sleep(bp, PRIBIO);
				}
				spl0();
				if (bp->b_flags & B_ERROR) {
					unkseg(va);
					return;
				}
			}
		} while (u.u_count);
		unkseg(va);
	} else {
		/*	The buffer is on a sector boundary
		**	but not necessarily on a page boundary.
		*/
	
		if ((bp->b_bcount = cc = 
			min( u.u_count, pgbnd(u.u_base))) < NBPP) {

			/*
			 *	Do the fragment of the buffer that's in the
			 *	first page
			 */

			bp->b_flags &= ~B_DONE;
			(*strat)(bp);
			spl6();
			while ((bp->b_flags & B_DONE) == 0) {
				bp->b_flags |= B_WANTED;
				sleep(bp, PRIBIO);
			}
			spl0();
			if (bp->b_flags & B_ERROR) {
				return;
			}
			bp->b_blkno += btod(cc);
			bp->b_un.b_addr += cc;
			u.u_count -= cc;
		}

		/*
		 *	Now do the DMA a page at a time
		 */

		while (u.u_count) {
			bp->b_bcount = cc = min(u.u_count, NBPP);
			bp->b_flags &= ~B_DONE;
			(*strat)(bp);
			spl6();
			while ((bp->b_flags & B_DONE) == 0) {
				bp->b_flags |= B_WANTED;
				sleep(bp, PRIBIO);
			}
			spl0();
			if (bp->b_flags & B_ERROR) {
				return;
			}
			bp->b_blkno += btod(cc);
			bp->b_un.b_addr += cc;
			u.u_count -= cc;
		}
	}
}  /* end dma_breakup */
