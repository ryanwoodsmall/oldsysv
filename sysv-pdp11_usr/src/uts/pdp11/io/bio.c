/* @(#)bio.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/var.h"

/*
 * swap IO headers.
 */
struct	buf	swbuf[NSWB];

#define	b_acount	b_resid

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	getablk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */


/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;

	sysinfo.lread++;
	bp = getblk(dev, blkno);
	if (bp->b_flags&B_DONE)
		return(bp);
	bp->b_flags |= B_READ;
	(*bdevsw[major(dev)].d_strategy)(bp);
	u.u_ior++;
	sysinfo.bread++;
	iowait(bp);
	return(bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *
breada(dev, blkno, rablkno)
dev_t dev;
daddr_t blkno, rablkno;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	if (!incore(dev, blkno)) {
		sysinfo.lread++;
		bp = getblk(dev, blkno);
		if ((bp->b_flags&B_DONE) == 0) {
			bp->b_flags |= B_READ;
			(*bdevsw[major(dev)].d_strategy)(bp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (rablkno && bfreelist.b_bcount>1 && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ|B_ASYNC;
			(*bdevsw[major(dev)].d_strategy)(rabp);
			u.u_ior++;
			sysinfo.bread++;
		}
	}
	if (bp == NULL)
		return(bread(dev, blkno));
	iowait(bp);
	return(bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
register struct buf *bp;
{
	register flag;

	sysinfo.lwrite++;
	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	(*bdevsw[major(bp->b_dev)].d_strategy)(bp);
	u.u_iow++;
	sysinfo.bwrite++;
	if ((flag&B_ASYNC) == 0) {
		iowait(bp);
		brelse(bp);
	} else if (flag & B_DELWRI)
		bp->b_flags |= B_AGE;
	else
		geterror(bp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
register struct buf *bp;
{

	sysinfo.lwrite++;
	bp->b_flags |= B_DELWRI | B_DONE;
	bp->b_resid = 0;
	brelse(bp);
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
register struct buf *bp;
{

	if (bfreelist.b_bcount>4)
		bp->b_flags |= B_ASYNC;
	bwrite(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
register struct buf *bp;
{
	register struct buf **backp;
	register s;

	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist.b_flags&B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&bfreelist);
	}
	if (bp->b_flags&B_ERROR) {
		bp->b_flags |= B_STALE|B_AGE;
		bp->b_flags &= ~(B_ERROR|B_DELWRI);
		bp->b_error = 0;
	}
	s = spl6();
	if (bp->b_flags & B_AGE) {
		backp = &bfreelist.av_forw;
		(*backp)->av_back = bp;
		bp->av_forw = *backp;
		*backp = bp;
		bp->av_back = &bfreelist;
	} else {
		backp = &bfreelist.av_back;
		(*backp)->av_forw = bp;
		bp->av_back = *backp;
		*backp = bp;
		bp->av_forw = &bfreelist;
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	bfreelist.b_bcount++;
	if (bp->b_paddr <= (paddr_t)(unsigned)sabuf[v.v_sabuf])
		bfreelist.b_acount++;
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	dp = bhash(dev, blkno);
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
		if (bp->b_blkno==blkno && bp->b_dev==dev && (bp->b_flags&B_STALE)==0)
			return(1);
	return(0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 */
struct buf *
getblk(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	if (major(dev) >= bdevcnt)
		panic("blkdev");

    loop:
	spl0();
	dp = bhash(dev, blkno);
	if (dp == NULL)
		panic("devtab");
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno!=blkno || bp->b_dev!=dev || bp->b_flags&B_STALE)
			continue;
		spl6();
		if (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			syswait.iowait++;
			sleep((caddr_t)bp, PRIBIO+1);
			syswait.iowait--;
			goto loop;
		}
		spl0();
		notavail(bp);
		return(bp);
	}
	spl6();
	if (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		sleep((caddr_t)&bfreelist, PRIBIO+1);
		goto loop;
	}
	spl0();
	notavail(bp = bfreelist.av_forw);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	return(bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 * and if flag non-zero, directly addressable
 */
struct buf *
getablk(flag)
{
	register struct buf *bp;
	register struct buf *dp;

loop:
	dp = &bfreelist;
	spl6();
	while (dp->av_forw == dp ||
		(flag && dp->b_acount == 0)) {
		dp->b_flags |= B_WANTED;
		sleep((caddr_t)dp, PRIBIO+1);
	}
	for (bp = dp->av_forw; flag; bp = bp->av_forw)
		if (bp->b_paddr <= (paddr_t)(unsigned)sabuf[v.v_sabuf])
			break;
	spl0();
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = B_BUSY|B_AGE;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = (dev_t)NODEV;
	return(bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
register struct buf *bp;
{

	syswait.iowait++;
	spl6();
	while ((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PRIBIO);
	spl0();
	syswait.iowait--;
	geterror(bp);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
notavail(bp)
register struct buf *bp;
{
	register s;

	s = spl6();
	bp->av_back->av_forw = bp->av_forw;
	bp->av_forw->av_back = bp->av_back;
	bp->b_flags |= B_BUSY;
	bfreelist.b_bcount--;
	if (bp->b_paddr <= (paddr_t)(unsigned)sabuf[v.v_sabuf])
		bfreelist.b_acount--;
	splx(s);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
register struct buf *bp;
{

	bp->b_flags |= B_DONE;
	if (bp->b_flags&B_ASYNC)
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}


/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
unsigned coreaddr;
{
	static struct buf *sbp;
	register struct buf *bp;
	register lkflg;

	syswait.swap++;
	if (sbp==NULL)
		sbp = &swbuf[0];
	bp = sbp++;
	if (sbp > &swbuf[NSWB-1])
		sbp = &swbuf[0];
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PSWP+1);
	}
	bp->b_flags = B_BUSY | B_PHYS | rdflg;
	bp->b_dev = swapdev;
	bp->b_bcount = ctob(count);
	bp->b_blkno = swplo+blkno;
	bp->b_paddr = (paddr_t)coreaddr << 6;
	/* lock process if not already locked */
	lkflg = (u.u_procp->p_flag&(SLOCK|SSYS))?0:SLOCK;
	u.u_procp->p_flag |= lkflg;
	(*bdevsw[major(swapdev)].d_strategy)(bp);
	u.u_iosw++;
	if (rdflg) {
		sysinfo.swapin++;
		sysinfo.bswapin += count;
	} else {
		sysinfo.swapout++;
		sysinfo.bswapout += count;
	}
	spl6();
	while((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PSWP);
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	spl0();
	u.u_procp->p_flag &= ~lkflg;
	bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
	if (bp->b_flags & B_ERROR)
		panic("IO err in swap");
	syswait.swap--;
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */
bflush(dev)
dev_t dev;
{
	register struct buf *bp;

loop:
	spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags |= B_ASYNC;
			notavail(bp);
			bwrite(bp);
			goto loop;
		}
	}
	spl0();
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer header, sometimes of a special type owned by the
 *	device, and sometimes from the physio pool of headers.
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{
	register unsigned base;
	register int nb;
	unsigned ts;
	int	hpf;

	base = (unsigned)u.u_base;
	/*
	 * Check odd base, odd count, and address wraparound
	 */
	if (base&01 || u.u_count&01 || base>=base+u.u_count)
		goto bad;
	/*
	 * Check overlap with text (ts and nb will be
	 * in 64-byte clicks).
	 */
	if (u.u_sep)
		ts = 0;
	else
		ts = (u.u_tsize + 0177) & ~0177;
	nb = base>>6;
	if (nb < ts)
		goto bad;
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 *
	 * If this fails, check whether the transfer is from the
	 * user's MAUS.  For now, it is only legal to raw I/O from
	 * one MAUS segment at a time with no spill over.
	 */
	if ((base + u.u_count - 1)>>6 >= ts + u.u_dsize
	&&  nb < 1024 - u.u_ssize) {
		ts = u.u_uisd[(nb>>7) + (u.u_sep?8:0)];
		if ((ts & RW) == 0 ||
		    (ts & RW) == RO  &&  rw == B_WRITE ||
		    ((ts>>8) & 0177) + 1 < btoc((long)(base & 017777) + u.u_count))
			goto bad;
	}
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;
	syswait.physio++;
	hpf = (bp == NULL);
	spl6();
	if (hpf) {
		while ((bp = pfreelist.av_forw) == NULL) {
			pfreelist.b_flags |= B_WANTED;
			sleep((caddr_t)&pfreelist, PRIBIO+1);
		}
		pfreelist.av_forw = bp->av_forw;
	} else while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO+1);
	}
	bp->b_error = 0;
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	ts = (u.u_sep? UDSA: UISA)->r[nb>>7];
	bp->b_paddr = ((paddr_t)ts << 6) + (base&017777);
	bp->b_blkno = u.u_offset >> BSHIFT;
	bp->b_bcount = u.u_count;
	u.u_procp->p_flag |= SLOCK;
	(*strat)(bp);
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep((caddr_t)bp, PRIBIO);
	u.u_procp->p_flag &= ~SLOCK;
	if (hpf) {
		bp->av_forw = pfreelist.av_forw;
		pfreelist.av_forw = bp;
		if (pfreelist.b_flags&B_WANTED) {
			pfreelist.b_flags &= ~B_WANTED;
			wakeup((caddr_t)&pfreelist);
		}
	} else if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (runin) {
		runin = 0;
		wakeup(&runin);
	}
	spl0();
	bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
	u.u_count = bp->b_resid;
	geterror(bp);
	syswait.physio--;
	return;
    bad:
	u.u_error = EFAULT;
}

physck(nblocks, rw)
daddr_t nblocks;
{
	register unsigned over;
	off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	limit = nblocks << BSHIFT;
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

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(bp)
register struct buf *bp;
{

	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}

/*
 * Invalidate blocks for a dev after last close.
 */
binval(dev)
{
	register struct buf *dp;
	register struct buf *bp;
	register i;

	for (i=0; i<v.v_hbuf; i++) {
		dp = (struct buf *)&hbuf[i];
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
			if (bp->b_dev == dev)
				bp->b_flags |= B_STALE|B_AGE;
	}
}
