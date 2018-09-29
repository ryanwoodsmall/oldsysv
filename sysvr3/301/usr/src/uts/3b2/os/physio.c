/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/physio.c	10.9"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5dir.h"
#include "sys/map.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"


/* Max. number of pages to swap per I/O */

#define NPAGE NPGPT

/*
 * swap I/O
 */

swap(pglptr, npage, rw)
register pglst_t	*pglptr;
{
	register buf_t	*bp;
	register dev_t	dev;
	register int	i;
	dbd_t		*dbd;
	int		ospl;

	ASSERT(syswait.swap >= 0);
	syswait.swap++;

	dbd = (dbd_t *)(pglptr->gp_ptptr + NPGPT);
	dev = swaptab[dbd->dbd_swpi].st_dev;

	ospl = spl6();
	while (pfreecnt == 0)
		sleep(&pfreelist, PRIBIO);
	ASSERT(pfreecnt);
	ASSERT(pfreelist.av_forw);
	pfreecnt--;
	bp = pfreelist.av_forw;
	pfreelist.av_forw = bp->av_forw;
	splx(ospl);

	bp->b_proc = u.u_procp;
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	bp->b_blkno = dbd->dbd_blkno;

	for (i = 0  ;  i < npage  ;  i++) {
		bp->b_un.b_addr =
			(caddr_t)ctob(pglptr++->gp_ptptr->pgm.pg_pfn);
		swapseg(dev, bp, 1, rw);
		bp->b_blkno += ptod(1);
	}

	ospl = spl6();
	bp->av_forw = pfreelist.av_forw;
	pfreelist.av_forw = bp;
	pfreecnt++;
	wakeup(&pfreelist);
	splx(ospl);

	ASSERT(syswait.swap);
	syswait.swap--;
}

swapseg(dev, bp, pg, rw)
dev_t		dev;
register struct buf *bp;
register int	pg;
{
	register int	ospl;

	u.u_iosw++;
	if (rw) {
		sysinfo.swapin++;
		sysinfo.bswapin += pg;
	} else {
		sysinfo.swapout++;
		sysinfo.bswapout += pg;
	}
	bp->b_bcount = ctob(pg);
	bp->b_flags &= ~B_DONE;
	(*bdevsw[bmajor(dev)].d_strategy)(bp);

	ospl = spl6();
	while ((bp->b_flags & B_DONE) == 0)
		sleep(bp, PRIBIO);
	splx(ospl);
	if (bp->b_flags & B_ERROR)
		cmn_err(CE_PANIC, "swapseg - i/o error in swap");
}


/*
 * Raw I/O. The arguments are
 * The strategy routine for the device
 * A buffer, which is usually NULL, or special buffer
 *   header owned exclusively by the device for this purpose
 * The device number
 * Read/write flag
 */

physio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{	register int		count;
	register caddr_t	base;
	register caddr_t	lbase;
	char			*kseg();
	register int		hpf;

	count = u.u_count;
	base = u.u_base;
	if (server()) {
		if ((lbase = kseg(btoc(count))) == 0)
			return;
		if (rw == B_WRITE) {
			if (remio(base, lbase, count)) {
				unkseg(lbase);
				return;
			}
			u.u_segflg = 1;	/*you're now dealing with kernel space*/
		}
		u.u_base = lbase;
	} else {
		if (userdma(base, count, rw) == NULL) {
			if (u.u_error == 0)
				u.u_error = EFAULT;
			return;
		}
	}
	ASSERT(syswait.physio >= 0);
	syswait.physio++;
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;


	hpf = (bp == NULL);
	if (hpf) {
		/*	Get a buffer header off of the
		 *	free list.
		 */

		spl6();
		while (pfreecnt == 0)
			sleep(&pfreelist, PRIBIO);
		ASSERT(pfreecnt);
		ASSERT(pfreelist.av_forw);
		pfreecnt--;
		bp = pfreelist.av_forw;
		pfreelist.av_forw = bp->av_forw;
		spl0();
	}

	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->b_un.b_addr = u.u_base;
	bp->b_blkno = btod(u.u_offset);
	bp->b_bcount = u.u_count;

	(*strat)(bp);

	spl6();
	while ((bp->b_flags & B_DONE) == 0) {
		sleep(bp, PRIBIO);
	}

	spl0();
	if (server()) {
		if (rw == B_READ) {
			u.u_segflg = 0;
			if (unremio(lbase, base, count))
				printf("unremio failed: err=%d\n",u.u_error);
		}
		unkseg(lbase);
	}
	else
		undma(base, count, rw);
	bp->b_flags &= ~(B_BUSY|B_PHYS);
	if (hpf) {
		spl6();
		bp->av_forw = pfreelist.av_forw;
		pfreelist.av_forw = bp;
		pfreecnt++;
		wakeup(&pfreelist);
		spl0();
	}

	geterror(bp);
	u.u_count = bp->b_resid;
	ASSERT(syswait.physio);
	syswait.physio--;
}


physck(nblocks, rw)
daddr_t nblocks;
{
	register unsigned over;
	register off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	limit = nblocks << SCTRSHFT;
	if (u.u_offset >= limit) {
		if (u.u_offset > limit || rw == B_WRITE)
			u.u_error = ENXIO;
		return(0);
	}
	upper = u.u_offset + u.u_count;
	if (upper > limit) {
		over = upper - limit;
		u.u_count -= over;
		uap = (struct a *)u.u_ap;
		uap->count -= over;
	}
	return(1);
}

