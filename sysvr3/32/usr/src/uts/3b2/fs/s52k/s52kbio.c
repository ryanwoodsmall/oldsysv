/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)3b2s52k:fs/s52k/s52kbio.c	10.1"
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5macros.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/conf.h"
#include "sys/var.h"
#include "sys/cmn_err.h"
#include "sys/inline.h"

/* The routines defined here provide support for the 2k file 
 * system, for the most part they are functionally equivalent
 * to the buffer routines found in bio.c. The differences are
 * limited to the knowledge of the different logical
 * block sizes.
 */


/* Convert logical block number to a physical number */
/* given block number and block size of the file system */
/* Assumes 512 byte blocks (see param.h). */

#define LTOPBLK(blkno, bsize)	(blkno * ((bsize>>SCTRSHFT)))

/* defined in bio.c, this is the count and wait flag for async writes.
   This is used to insure all async writes complete before the root
   file system is umounted. 
 */

extern int basyncnt, basynwait;

/* set buffer state to busy */

#define s52kbnotavail(bp)\
{\
	register s;\
	s = spl6();\
	bp->av_back->av_forw = bp->av_forw;\
	bp->av_forw->av_back = bp->av_back;\
	bp->b_flags |= B_BUSY;\
	s52kflist.b_bcount--;\
	splx(s);\
}

/* hash routine for fast access to cached buffers */
 
#define s52kbhash(d,b)	((struct buf *)&s52khbuf[((int)d+(int)b)&(s52khash-1)])

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
 *	s52kgetblk
 *	s52kbread
 *	s52kbreada
 *	s52kgeteblk (buffer not assoicated with a disk block)
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	s52kbwrite
 *	s52kbdwrite
 *	s52kbawrite
 *	s52kbrelse
 */


/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
*/

struct buf *
s52kbreada(dev, blkno, rablkno, bsize)
register dev_t dev;
daddr_t blkno, rablkno;
long bsize;
{
register struct buf *bp, *rabp;
struct buf *s52kgetblk();
struct buf *s52kbread();
 	bp = NULL;
 	if (!s52kbincore(dev, blkno, bsize)) {
 		sysinfo.lread++;
 		bp = s52kgetblk(dev, blkno, bsize);
 		if ((bp->b_flags&B_DONE) == 0) {
 			bp->b_flags |= B_READ;
 			bp->b_bcount = bsize;
 			(*bdevsw[bmajor(dev)].d_strategy)(bp);
 			u.u_ior++;
 			sysinfo.bread++;
 		}
 	}
 	if (rablkno && s52kflist.b_bcount>1 && !s52kbincore(dev, rablkno, bsize)) {
 		rabp = s52kgetblk(dev, rablkno, bsize);
 		if (rabp->b_flags & B_DONE)
 			s52kbrelse(rabp);
 		else {
 			rabp->b_flags |= B_READ|B_ASYNC;
 			rabp->b_bcount = bsize;
 			(*bdevsw[bmajor(dev)].d_strategy)(rabp);
 			u.u_ior++;
 			sysinfo.bread++;
 		}
 	}
 	if (bp == NULL)
 		return(s52kbread(dev, blkno, bsize));
 	iowait(bp);
 	return(bp);
 }

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */

s52kbincore(dev, blkno, bsize)
register dev_t dev;
register daddr_t blkno;
register long bsize;
{
	register struct buf *bp;
 	register struct buf *dp;
 
 	blkno = LTOPBLK(blkno,bsize) ;
 	dp = s52kbhash(dev, blkno);
 	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw)
 		if (bp->b_blkno==blkno && bp->b_dev==dev && (bp->b_flags&B_STALE)==0)
 			return(1);
 	return(0);
}
 
/*
* get an empty block,
* not assigned to any particular device
*/
struct buf *
s52kgeteblk()
{
 	register struct buf *bp;
 	register struct buf *dp;
 
 loop:
 	spl6();
 	while (s52kflist.av_forw == &s52kflist) {
 		s52kflist.b_flags |= B_WANTED;
 		sleep((caddr_t)&s52kflist, PRIBIO+1);
 	}
 	spl0();
 	dp = &s52kflist;
 	bp = s52kflist.av_forw;
 	s52kbnotavail(bp);
 	if (bp->b_flags & B_DELWRI) {
 		bp->b_flags |= B_ASYNC;
 		s52kbwrite(bp);
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
 	bp->b_bcount = SBUFSIZE;
 	return(bp);
}
 
/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * from closef and update
 */

s52kbflush(dev)
register dev_t dev;
{
	register struct buf *bp;
 
loop:
	spl6();
	for (bp = s52kflist.av_forw; bp != &s52kflist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			bp->b_flags |= B_ASYNC;
			s52kbnotavail(bp);
			s52kbwrite(bp);
			spl0();
			goto loop;
		}
	}
}

s52kbdflush()
{
	register struct buf *bp;
	register autoup;

	autoup = (v.v_autoup * HZ);

loop:

	spl6();
	for (bp = s52kflist.av_forw; bp != &s52kflist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && ((lbolt-bp->b_start) >= autoup)) {
			bp->b_flags |= B_ASYNC;
			s52kbnotavail(bp);
			s52kbwrite(bp);
			spl0();
			goto loop;
		}
	}
}

/*
 * Invalidate blocks for a dev after last close.
 */

s52kbinval(dev)
register dev_t dev;
{
	register struct buf *dp;
 	register struct buf *bp;
 	register i;
 
 	for (i=0; i<s52khash; i++) {
 		dp = (struct buf *)&s52khbuf[i];
 		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
 			if (bp->b_dev == dev)
 				bp->b_flags |= B_STALE|B_AGE;
 	}
}
 
struct buf *
s52kgetblk(dev, blkno, bsize)
dev_t dev;
daddr_t blkno;
long bsize;
{
 	register struct buf *bp;
 	register struct buf *dp;
 
 	if (bmajor(dev) >= bdevcnt)
 		cmn_err(CE_PANIC,"blkdev");
 
 	blkno = LTOPBLK(blkno,bsize);
     loop:
 	spl0();
 	if ((dp = s52kbhash(dev,blkno)) == NULL)
 		cmn_err(CE_PANIC,"devtab");
 	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
 		if (bp->b_blkno!=blkno || bp->b_dev!=dev || bp->b_flags&B_STALE)
 			continue;
 		spl6();
 		if (bp->b_flags&B_BUSY) {
 			bp->b_flags |= B_WANTED;
 			syswait.iowait++;
 			bp->b_flags |= B_S52K;		/* set B_S52K for iodone */
 			sleep((caddr_t)bp, PRIBIO+1);
 			syswait.iowait--;
 			goto loop;
 		}
 		spl0();
 		s52kbnotavail(bp);
 		return(bp);
 	}
 	spl6();
 	if (s52kflist.av_forw == &s52kflist) {
 		s52kflist.b_flags |= B_WANTED;
 		sleep((caddr_t)&s52kflist, PRIBIO+1);
 		goto loop;
 	}
 	spl0();
 	bp = s52kflist.av_forw;
 	s52kbnotavail(bp);
 	if (bp->b_flags & B_DELWRI) {
 		bp->b_flags |= B_ASYNC;
 		s52kbwrite(bp);
 		goto loop;
 	}
 	bp->b_flags = B_BUSY|B_S52K;		/* set B_S52K for iodone */
 	bp->b_back->b_forw = bp->b_forw;
 	bp->b_forw->b_back = bp->b_back;
 	bp->b_forw = dp->b_forw;
 	bp->b_back = dp;
 	dp->b_forw->b_back = bp;
 	dp->b_forw = bp;
 	bp->b_dev = dev;
 	bp->b_blkno = blkno;
 	bp->b_bcount = bsize;
 	return(bp);
}
 
struct buf *
s52kbread(dev, blkno, bsize)
dev_t dev;
daddr_t blkno;
long bsize;
{
 	struct buf *bp;
 	struct buf *s52kgetblk();
 
 	bp = s52kgetblk(dev, blkno, bsize);
 
 	if(bp->b_flags & B_DONE)
 		return(bp);
 	bp->b_flags |= B_READ;
 	bp->b_bcount = bsize;
 	(*bdevsw[bmajor(dev)].d_strategy)(bp);
 	u.u_ior++;
 	sysinfo.bread++;
 	iowait(bp);
 	return(bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */

s52kbwrite(bp)
register struct buf *bp;
{
 	register flag;
 
 	sysinfo.lwrite++;
 	flag = bp->b_flags;
 	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
 	(*bdevsw[bmajor(bp->b_dev)].d_strategy)(bp);
 	u.u_iow++;
 	sysinfo.bwrite++;
 	if ((flag&B_ASYNC) == 0) {
 		iowait(bp);
 		s52kbrelse(bp);
 	} else {
		basyncnt++;
 		if (!(flag & B_DELWRI))
 			geterror(bp);
 	}
}
 
/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * Also save the time that the block is first marked as delayed
 * so that it will be written in a reasonable time.
 */

s52kbdwrite(bp)
register struct buf *bp;
{
 	sysinfo.lwrite++;
 	if (!(bp->b_flags & B_DELWRI))
 		bp->b_start = lbolt;
 	bp->b_flags |= B_DELWRI | B_DONE;
 	bp->b_resid = 0;
 	s52kbrelse(bp);
}
 
/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */

s52kbawrite(bp)
register struct buf *bp;
{
 
 	if (s52kflist.b_bcount>4)
 		bp->b_flags |= B_ASYNC;
 	s52kbwrite(bp);
 }
 
s52kbrelse(bp)
struct buf *bp;
{
 
 	register s;
 	register struct buf **backp;
 
 	if (bp->b_flags&B_WANTED)
 		wakeup((caddr_t)bp);
 	if (s52kflist.b_flags&B_WANTED) {
 		s52kflist.b_flags &= ~B_WANTED;
 		wakeup((caddr_t)&s52kflist);
 	}
 	if (bp->b_flags&B_ERROR)  {
 		bp->b_flags |= B_STALE|B_AGE;
	 	bp->b_flags &= ~(B_ERROR|B_DELWRI);
 		bp->b_error = 0;
	}
 	s=spl6();
 	if (bp->b_flags & B_AGE) {
 		backp = &s52kflist.av_forw;
 		(*backp)->av_back = bp;
 		bp->av_forw = *backp;
 		*backp = bp;
 		bp->av_back = &s52kflist;
 	} else {
 		backp = &s52kflist.av_back;
 		(*backp)->av_forw = bp;
 		bp->av_back = *backp;
 		*backp = bp;
 		bp->av_forw = &s52kflist;
 	}
 	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
 	s52kflist.b_bcount++;
	bp->b_reltime = (unsigned long)lbolt;
 	splx(s);
}

struct buf s52kflist;		/* 2K buffer freelist */


/* This routine initializes the 2KB buffer pool */
s52kbinit()
{
	register struct buf *bp;
	register struct buf *dp;
	register unsigned i;
	register caddr_t pbuffer;
	extern int s52kfreecnt;
	extern int s52khash;

	/* initialize 2KB buffer pool */

	dp = &s52kflist;
	dp->b_forw = dp->b_back =
	    dp->av_forw = dp->av_back = dp;
	pbuffer = (caddr_t)s52kbuffers;
	pbuffer += BSIZE-1;	/* align buff cache on block boundry */
	pbuffer = (caddr_t)((int)pbuffer & ~(BSIZE -1));

	for (i=0, bp=s52kbuf; i<s52kfreecnt; i++,bp++,pbuffer += SBUFSIZE) {
		bp->b_dev = NODEV;
		bp->b_un.b_addr = pbuffer;
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_bcount = 0;
		s52kbrelse(bp);
	}

	for (i=0; i < s52khash; i++)
		s52khbuf[i].b_forw = s52khbuf[i].b_back = (struct buf *)&s52khbuf[i];
}
