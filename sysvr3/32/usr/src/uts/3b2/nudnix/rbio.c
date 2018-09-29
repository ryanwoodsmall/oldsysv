/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rbio.c	10.1.3.7"
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
#include "sys/comm.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"
#include "sys/rbuf.h"


unsigned	long	rcache_time;	/* adjusted cache disable time (in HZ)*/

/*
 * rcache_out copies the contents of a linked list of RFS buffers
 * from the buffer pool to process data area
 */

uint rcache_out(rbp,sd)
register struct rbuf *rbp;
sndd_t sd;
{
 	register struct rbuf *bp;
	register struct rbuf *tmp_bp;
	long n,on;

	ASSERT(rbp != NULL);
	ASSERT(sd->sd_count > 0);
	/*
	 * Copyout each buffer, first computing offset 
	 * within buffer and byte count 
	 */
	on = (sd->sd_offset & RBMASK);
	if ((n = (RBSIZE - on)) > sd->sd_count)
		n = sd->sd_count;
	bp = rbp;
	do  {
		if (u.u_segflg == 1) 
			bcopy(bp->b_un.b_addr+on, u.u_base, n);
		else if (copyout(bp->b_un.b_addr+on, u.u_base, n)) {
				u.u_error = EFAULT;
				goto out;
		}
		sd->sd_count -= n;
		sd->sd_offset += n;
		u.u_base += n;
		if ( sd->sd_count != 0  )
		{
			on = 0;
			n = (sd->sd_count > RBSIZE)?RBSIZE:sd->sd_count;
		}
 		bp = bp->av_forw;
	} while ( bp != rbp);
out:
	rbrelse(rbp);
	return(sd->sd_count);
}


/*
 * rcache_in copies data from process data area to buffer pool
 */

rcache_in(rbp, sd, from, count)
register struct rbuf *rbp;
register sndd_t sd;
register char *from;
register int count;
{
	register struct rbuf *bp;
	register struct rbuf *tmp_bp;
	long n,on;

	ASSERT(rbp != NULL);

	on = (sd->sd_offset & RBMASK);
	if ((n = (RBSIZE - on)) > count)
		n = count;
	bp = rbp;
	do {
		bcopy(from, bp->b_un.b_addr+on, n);
		count -= n;
		sd->sd_offset += n;
		from += n;
		if ( count != 0  ) {
			on = 0;
			n = (count > RBSIZE)?RBSIZE:count;
		}
		bp = bp->av_forw;
	} while (  bp != rbp);
	rbrelse(rbp);
}

/*
 * Release one or more buffers (assumes freelist pointers (av_forw and
 * and av_back) form circular linked list of buffers to be released. 
 * Single buffer must therefore point to itself (rgeteblk and rget_cache 
 * set things up this way).
 */
rbrelse(rbp)
register struct rbuf *rbp;
{
	register struct rbuf *bp;
	register struct rbuf *nextbp;
	register struct rbuf **backp;
	register s;

	bp = rbp;
   	do {
		ASSERT(!(bp->b_flags&B_WANTED));
		nextbp = bp->av_forw;
		/* Wake-up processes waiting for buffer for local access */
		if (bfreelist.b_flags&B_WANTED) {
			bfreelist.b_flags &= ~B_WANTED;
			wakeup((caddr_t)&bfreelist);
		}
		if (bp->b_flags & B_AGE) {
			backp = &rbfreelist.av_forw;
			(*backp)->av_back = bp;
			bp->av_forw = *backp;
			*backp = bp;
			bp->av_back = &rbfreelist;
		} else {
			backp = &rbfreelist.av_back;
			(*backp)->av_forw = bp;
			bp->av_back = *backp;
			*backp = bp;
			bp->av_forw = &rbfreelist;
		}

		bp->b_flags &= ~(B_WANTED|B_BUSY|B_AGE);
		s = spl6();
		bp->b_reltime = (unsigned long)lbolt;
		splx(s);
	} while (( bp = nextbp) != rbp);
}


/*
 * Search cache for one or more blocks containing remote file data.
 * If all blocks are found, return linked list of blocks.  Otherwise
 * return NULL if even one block is not found.
 */
struct rbuf *
rget_cache(sd, blkno, blkct)
register sndd_t sd;
register daddr_t blkno;
register int blkct;
{
	register struct rbuf *bp;
	register struct rbuf *dp;
	register struct rbuf *rbp;
	register found; 
	int inval_write = 0;

	ASSERT(blkct != 0);
	for (rbp = NULL ;blkct > 0; blkct--,blkno++) {
		found = 0;
		dp = rbhash(sd->sd_fhandle,blkno);
		ASSERT(dp != NULL);
		/*
 		 * Set up buffer linked list for read request.  Add each blkno
		 * to end of list before looking for the next.
 		 */
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if (bp->b_blkno==blkno && bp->b_fhandle==sd->sd_fhandle
			   && bp->b_mntindx==sd->sd_mntindx 
			   && bp->b_queue==sd->sd_queue 
		           && !(bp->b_flags&B_STALE)) {
				ASSERT(!(bp->b_flags & B_BUSY));
				remnotavail(bp);
				found = 1;
	
				if (rbp != NULL) {
					bp->av_back = rbp->av_back;
					bp->av_forw = rbp;
					rbp->av_back->av_forw = bp;
					rbp->av_back = bp;
				}
				else {
					bp->av_forw = bp;
					bp->av_back = bp;
					rbp = bp;
				}
				break;
			}
		}
		/* 
		 * If there's even one missed buffer, terminate search.
		 * (Unless we're doing a multi-block write, in which case we 
		 * have to get remaining blocks in order to invalidate them).
		 */
		if (found == 0) {
			if (u.u_syscall == DUWRITE || u.u_syscall == DUWRITEI)
				inval_write = 1;
			else 
				break;
		}
	}
	/* if you've found all buffers, return immediately */
	if ((bp = rbp) != NULL) {
		if (blkct == 0 && inval_write == 0)
			return(rbp);
		if (inval_write)
			do {
				bp->b_flags |= B_STALE|B_AGE;
				bp = bp->av_forw;
			} while (bp != rbp);
		/* if you don't get everything, release each buffer*/
		rbrelse(rbp);
	}
	return(NULL);
}


/*
 *	Get an empty block from buffer cache
 *	and hash it to the client cache
 */

struct rbuf *
rgeteblk(sd, bno)
register sndd_t sd;
{
	register struct buf *bp;
	register struct rbuf *rbp;
	register struct rbuf *dp;
	register rswitch;
	register s;

   loop:

	/*   Check if buffer is to be taken from the remote list. */
	if ( (rbfreelist.av_forw != &rbfreelist)
	   && (rbp = chk_rlist(RGET)) != NULL)
		goto found;

	/*
	 * If you can't get from remote freelist, check if local freelist
	 * buffer is available.  If not, return no buffer available.
	 */
	s = spl6();
        if (( (bp = bfreelist.av_forw) == &bfreelist) ||  (lbuf_ct <=  nlbuf)) {
		splx(s);
		return(NULL);
	}
	splx(s);
	/*   Else get a block from the local list. */
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bp->b_flags = 0;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	lbuf_ct--;
	rbuf_ct++;
	rbp = (struct rbuf *)bp;

   found:
	DUPRINT5(DB_CACHE,"bp %x rbp %x rbufct %x, lbufct %x\n",bp,rbp,rbuf_ct,lbuf_ct);
	rbp->b_flags = B_BUSY|B_REMOTE;
	/*
	 * Set freelist pointers to buffer address (rget_cache and rbrelse 
	 * need this).
	 */
	rbp->av_forw = rbp->av_back = rbp;
	rbp->b_fhandle = sd->sd_fhandle;
	rbp->b_mntindx = sd->sd_mntindx;
	rbp->b_queue = sd->sd_queue;
	rbp->b_blkno = bno;
	rbp->b_vcode = 0;
	rbp->b_reltime = 0;
	/*
	 * Link onto hash (fhandle and block number) queue 
	*/
	dp = rbhash(rbp->b_fhandle, rbp->b_blkno);
	rbp->b_forw = dp->b_forw;
	rbp->b_back = dp;
	dp->b_forw->b_back = rbp;
	dp->b_forw = rbp;
	/*
	 * Link onto file (fhandle) queue 
	*/
	dp = rfhash(rbp->b_fhandle);
	rbp->f_forw = dp->f_forw;
	rbp->f_back = dp;
	rbp->f_forw->f_back = rbp;
	dp->f_forw = rbp;
	return(rbp);
}

/*
 * Save vcode in buf headers for file
 */
rfbufstamp(sd, vcode)
register sndd_t sd;
register unsigned long vcode;
{
	register struct rbuf *dp;
	register struct rbuf *bp;


	dp = rfhash(sd->sd_fhandle);
	ASSERT(dp != NULL);
	for (bp=dp->f_forw; bp != dp; bp = bp->f_forw) {
		if ( bp->b_fhandle!=sd->sd_fhandle
			|| bp->b_mntindx!=sd->sd_mntindx 
			|| bp->b_queue!=sd->sd_queue || (bp->b_flags&B_STALE))
			{
				continue;
			}
			bp->b_vcode = vcode;
	}
}
/*
 * Invalidate buffers for a remote file 
 */
rfinval(sd, blkno, blkct )
register sndd_t sd;
register daddr_t blkno;
register int blkct;
{
	register struct rbuf *dp;
	register struct rbuf *bp;

	if (!(rcacheinit))
		return;
	dp = rfhash(sd->sd_fhandle);
	ASSERT(dp != NULL);
	rcinfo.inval++;
	for (bp=dp->f_forw; bp != dp; bp = bp->f_forw) {
		if ( bp->b_fhandle!=sd->sd_fhandle
			|| bp->b_mntindx!=sd->sd_mntindx 
			|| bp->b_queue!=sd->sd_queue || (bp->b_flags&B_STALE))
			{
				continue;
			}
			if ( (blkno == -1) || ((bp->b_blkno < blkno + blkct)
			   && (bp->b_blkno >= blkno))) {
				bp->b_flags |= B_STALE|B_AGE;
				rcinfo.blk_inval++;
			}
	}
}
/*
 * Invalidate out-of-date buffers for a remote file at first open
 * or when set read/write lock
 */
rfchk_vcode(sd, vcode)
register sndd_t sd;
register unsigned long vcode;
{
	register struct rbuf *dp;
	register struct rbuf *bp;

	dp = rfhash(sd->sd_fhandle);
	ASSERT(dp != NULL);
	rcinfo.inval++;
	for (bp=dp->f_forw; bp != dp; bp = bp->f_forw) {
		if ( bp->b_fhandle!=sd->sd_fhandle
			|| bp->b_mntindx!=sd->sd_mntindx 
			|| bp->b_queue!=sd->sd_queue || (bp->b_flags&B_STALE))
			{
				continue;
			}
		if ((bp->b_vcode != vcode) || vcode == 0) {
			bp->b_flags |= B_STALE|B_AGE;
			rcinfo.blk_inval++;
		}
	}
}

/*
 * Invalidate blocks for a remote advertised directory at unmount.
 */
rmntinval(sd)
register sndd_t sd;
{
	register struct rbuf *dp;
	register struct rbuf *bp;
	register i;


	for (i=0; i<nrhbuf; i++) {
		dp = (struct rbuf *)&rhbuf[i];
		for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
			if ( bp->b_mntindx!=sd->sd_mntindx 
				|| bp->b_queue!=sd->sd_queue || (bp->b_flags&B_STALE))
			{
				continue;
			}
			bp->b_flags |= B_STALE|B_AGE;
		}
	}
}

/*
 * Initialize the buffer I/O system for sharing of buffers between
 * local disk and remote RFS access.  Set all RFS hash buffer lists to empty.
 */

rb_init()

{
	register unsigned i;
	struct	rbuf	*rdp;

	/* Initialization  for first RFS start only*/
	if ((rcacheinit == 0) && (rc_time != -1) && (nrhbuf > 0)) {
		rcache_enable = 1;
		rcacheinit = 1;
		maxbufage = BUFAGE * HZ;
		rcache_time = ((unsigned long) rc_time) * HZ;
		/*
		 * Check that minimum number of local and remote buffers
		 * does not exceed total number of buffers.
		 */
		if (nlocal + nremote > v.v_buf) {
			nlocal = nremote = v.v_buf/3;
			cmn_err(CE_WARN, "Sum of NLOCAL and NREMOTE exceeds NBUF");
		}
		/*
		 * If NLOCAL or NREMOTE set to zero, use default number of 
		 * buffers.
		 */
		if ((nlbuf = nlocal) == 0)
			nlbuf = v.v_buf/3;
		if ((nrbuf = nremote) == 0)
			nrbuf = v.v_buf/3;
		rdp = &rbfreelist;
		rdp->b_forw = rdp->b_back = rdp->av_forw = 
			rdp->av_back = rdp->f_forw = rdp->f_back = rdp;
		lbuf_ct = v.v_buf;
		rbuf_ct = 0;
		for (i=0; i < nrhbuf; i++) {
			rhbuf[i].b_forw = rhbuf[i].b_back = rhbuf[i].f_forw = 
				rhbuf[i].f_back = (struct rbuf *)&rhbuf[i];
		}
	}
}

/*
 * Fill buffers with data obtained from reads of RFS regular files on the
 * server.  
 */
fill_rbuf(bpp,sd,resp,rbuf_off)
struct rbuf **bpp;
register sndd_t sd;
struct response *resp;
int *rbuf_off;
{
	register struct	rbuf *rbp;
	int count, on, n, bno;
	char *from;

	from = (char *) resp->rp_data;
	count = resp->rp_count;
	rbp = *bpp;
	on = *rbuf_off;
	bno = off_to_blkoff(sd->sd_offset);
   	do {
		if ((n = (RBSIZE-on)) > count)
			n = count;
		/*
		 * if you're starting a new buffer and can't find it in the
		 * cache, get an empty block and begin filling buffer. If
		 * it's already in cache, no need to copy to it.  (This can 
		 * happen for multi-block read requests, where the absence of 
		 * even one block causes the whole request to be sent, although
		 * the other blocks are in cache). 
		 */
		if ((rbp == NULL) && (on == 0 )) {
			struct rbuf *tmp_rbp;
		   	if ((tmp_rbp=rget_cache(sd, bno,1)) == NULL) 
				rbp = rgeteblk(sd, bno);
			else
				rbrelse(tmp_rbp);
		}
		/*
		 * If you're in the middle of filling a buffer obtained 
		 * by rgeteblk, always copy to it.  Check if buffer pointer
		 * is null (meaning buffer is already in cache or rgeteblk
		 * couldn't get a buffer).
		 */
		if (rbp != NULL)
			bcopy(from, rbp->b_un.b_addr+on, n);
		from += n;		
		count -= n;

		/*  if we've filled up a buffer, release it */
		if ( (((on + n) & RBMASK) == 0) || ((resp->rp_opcode == DUREAD 
		   || resp->rp_opcode == DUREADI) && (count == 0)) ) {
			if (rbp) {
				rbrelse(rbp);
				rbp = NULL;
			}
			on = 0;
			bno++;
		}
		else	{
			on += n;
		}
	} while (count > 0);
	*rbuf_off = on;
	*bpp = rbp;
}
/* 
 * Buffers can be taken from local or remote (network) cache freelist.
 * First check for network caching configured and non-empty remote
 * freelist, to see if there is something there to be taken.  If 
 * there is, choose which freelist to take from, according to 
 * following decision sequence.
 *
 * 	(1) if 1st remote buffer is B_STALE or B_AGE, or local has no more than 
 *		its guaranteed minimum of buffers (tunable parameter NLOCAL)
 *		always take from remote list.
 *
 * 	(2) or if the network is using more than its guaranteed minimum,
 *		or the first remote buffer is old (>maxbufage), or
 *	 	lbolt has cycled around, then compare first 
 * 		remote freelist buffer with first local freelist buffer 
 *		(remote buffer will be taken if it's older). 
 */

struct rbuf *chk_rlist(bufaccess) 
register int bufaccess;
{
	register  struct buf  *bp;
	register  struct rbuf *rbp;
	register int s;

	ASSERT(rcacheinit != 0);
	ASSERT(rbfreelist.av_forw != &rbfreelist);
	rbp = rbfreelist.av_forw;
	s = spl6();
	if (  (rbp->b_flags & (B_STALE|B_AGE)) || (lbuf_ct <= nlbuf) 
	   ||	( ( (rbuf_ct > nrbuf)
	   	|| ((unsigned long)lbolt - rbp->b_reltime > maxbufage)
	   	|| ((unsigned long)lbolt < rbp->b_reltime) )
		&& ( ((bp = bfreelist.av_forw) != &bfreelist)
	           && (rbp->b_reltime < bp->b_reltime) ) ) ) {
			remnotavail(rbp);
			/* unlink from block and file hash tables */
			rbp->b_back->b_forw = rbp->b_forw;
			rbp->b_forw->b_back = rbp->b_back;
			rbp->f_back->f_forw = rbp->f_forw;
		 	rbp->f_forw->f_back = rbp->f_back;
			if (bufaccess == LGET) {
				rbp->b_flags = 0;
				rbp->b_forw = rbp->b_back = rbp->f_forw = 0;
				rbp->f_back = rbp->av_forw = rbp->av_back = 0;
				rbp->b_fhandle = rbp->b_mntindx = 0;
				rbp->b_queue = 0;
				rbp->b_vcode = rbp->b_reltime = rbp->b_blkno = 0;
				/* Adjust buffer usage counters */
				lbuf_ct++;
				rbuf_ct--;
			}
	}
	else
		rbp = NULL;
	splx(s);
	return(rbp);
}
