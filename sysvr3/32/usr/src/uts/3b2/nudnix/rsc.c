/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rsc.c	10.19.7.1"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5macros.h"
#include "sys/buf.h"
#include "sys/rbuf.h"
#include "sys/sysinfo.h"
#include "sys/hetero.h"

extern	rcvd_t	cr_rcvd ();
extern	mblk_t	*dupmsg();
extern	mblk_t	*reusebuf();
extern	mblk_t	*getcbp();


/*
 * remote system call
 *
 *	Send a request to the server and wait for the response.
 *	Handle intermediate responses (ACK, NACK, data movement).
 *	This routine should never be called by server.
 *
 *	resp and gift are returned as side effects.
 */
rsc (sd, rq_bp, req_size, rp_bp, gift)
sndd_t	sd;			/* which send descriptor to send on	*/
sndd_t	gift;			/* gift back from the server		*/
mblk_t	*rq_bp;			/* incoming request block ptr		*/
mblk_t	**rp_bp;		/* ptr to response back from server	*/
int	req_size;		/* how many bytes			*/
{
	mblk_t	*nbp;
	struct	response *resp;	/* the response msg body		*/
	struct	response *nresp;
	struct message *smsg;
	rcvd_t  rd;	/* where to receive the response	*/
	int	unclean = TRUE;	/* need to free send buffer		*/
	int	ret = 0;	/* return from sndmsg			*/
	int	size, i;	/* ignored 				*/
	int	count = 0;	/* saves #bytes left to be copied	*/
	int	on, frontgap, req_count; /*RFS cache parameters */
	off_t	req_offset;
	caddr_t	save_ubase;
	struct rbuf *rbp = NULL;

	if ((rd = cr_rcvd (FILE_QSIZE, SPECIFIC)) == NULL) {
		u.u_error = ENOMEM;
		freemsg (rq_bp);
		u.u_procp->p_flag &= ~SRSIG;	/*clear any remote signal flags*/	
		return (FAILURE);
	}

	/* keep track of sd we're going out on (client side only) */
	rd->rd_inode = (struct inode *) sd;
	u.u_procp->p_minwd = (struct rcvd *) sd;	/* for remote signal */
	/*
	 * for RFS caching, adjust read size so that reads of
	 * whole blocks are sent to the server.
	 */
	if ((u.u_rcstat & U_RCACHE) && (u.u_syscall == DUREAD ||
			u.u_syscall == DUREADI) ){
		register struct	request	*req;
		on = 0;
		save_ubase = u.u_base;
		req = (struct request *)PTOMSG(rq_bp->b_rptr);
		req_offset = req->rq_offset = off_to_blkoff(u.u_offset)<<RBSHIFT;
		req_count = req->rq_count = 
			ct_to_blkct(u.u_count,u.u_offset)<<RBSHIFT;
		frontgap = u.u_offset - req->rq_offset;
	}

	if ((ret = sndmsg (sd, rq_bp, req_size, rd)) != SUCCESS)
		goto out;

getit:	if (de_queue (rd, rp_bp, gift, &size) == FAILURE) {
		u.u_error = ECOMM;
		ret = FAILURE;
		goto out;
	}
	if ((sd->sd_stat & SDLINKDOWN) && gift) {
		DUPRINT1(DB_RECOVER, "rsc: sd went bad during de_queue \n");
		freemsg (*rp_bp);
		u.u_error = ENOLINK;
		ret = FAILURE;
		goto out;
	}
	resp = (struct response *) PTOMSG((*rp_bp)->b_rptr);

	switch (resp->rp_opcode)  {

	/* move data from kernel on remote machine to user on this machine */
	case DUREAD:
	case DUREADI:
		if (u.u_error && count > 0) /*error happened in copyout*/
			resp->rp_rval = count; 
		if(resp->rp_subyte)
			break;
	case DUCOPYOUT:
		if (u.u_error == 0 && resp->rp_errno == 0 && u.u_syscall == DUGETDENTS) {
			if (GDP(sd->sd_queue)->hetero != NO_CONV) {
				i = denfcanon(resp->rp_count, resp->rp_data, resp->rp_data);
				if (i)
					resp->rp_count = i;
			}
		}
		if (u.u_error == 0 || resp->rp_errno == 0){
			if (u.u_rcstat & U_RCACHE) { 
				int	ucount, adjust;
				char *from;

				/*
				 * Fill RFS cache buffer with read data.
				 * Since this may take more than one copyout
				 * (if copyout size != client buffer size),
				 * we may have to hold the buffer between
				 * copyout messages.
				 */
				fill_rbuf(&rbp,sd,resp,&on);

				/* Copy data to user space.  (Response count
				 * must be adjusted because the copyout returns 
				 * whole buffers). */

				ucount = resp->rp_count;
				from = resp->rp_data;
				
				/*
				 * Adjust "frontgap" difference (difference
				 * between start of whole buffer(s) request
				 * to server (req_offset) and start of what
				 * the process actually wants to read 
				 * (u.u_offset).  
				 */
				if ((adjust = u.u_offset - req_offset) > 0) {
					from += adjust;
					ucount -= adjust;
				}
				req_offset += resp->rp_count;
				/*
				 * Adjust "endgap" difference (difference
				 * between end of whole buffer(s) request
				 * to server and end of what the
				 * process actually wants to read 
				 */
				if (ucount > sd->sd_count) 
					ucount = sd->sd_count;
				/* Copy to user space */
				if (ucount >  0) {
					if (u.u_segflg == 1) 
						bcopy(from, u.u_base, ucount);
					else if (copyout(from,u.u_base,ucount)){
						u.u_error = EFAULT;
						count = (save_ubase + u.u_count) - u.u_base;
					}
					if (u.u_error == 0) {
						u.u_base += ucount;
						sd->sd_offset += ucount;
						sd->sd_count -= ucount;
					}
				}
				/*
				 * Adjust return value to reflect 
				 * original client adjustment of read 
				 * request to cache block boundaries.
				 */
				if ((resp->rp_opcode == DUREAD 
				   || resp->rp_opcode == DUREADI) ) 
					resp->rp_rval = (save_ubase + u.u_count) - u.u_base;
			}
			else if (u.u_syscall == DUREADI && u.u_segflg == 1)
					bcopy (resp->rp_data, 
						resp->rp_bufptr,resp->rp_count);
			else 
				if (copyout (resp->rp_data,	
					resp->rp_bufptr,resp->rp_count)){
					u.u_error = EFAULT;
					count = (u.u_base+u.u_count) - (char *)resp->rp_bufptr;
					resp->rp_rval = count;
				}
		}
		if (resp->rp_opcode == DUCOPYOUT)  {
			/*  check if a response should be	
			 *  send back so that the
			 *  other end can do more copyout
			 */
			if (resp->rp_copysync) {
				nbp = getcbp(sizeof(struct response)-DATASIZE);
				nresp = (struct response *) PTOMSG(nbp->b_rptr);
				nresp->rp_type = RESP_MSG;
				nresp->rp_opcode = DUCOPYOUT;
				if(sndmsg (gift, nbp, sizeof(struct response)
					-DATASIZE, (rcvd_t)NULL) == FAILURE)  {
					printf("rsc: copyout sndmsg on sd %x fail, remote syscall may hang\n",gift);
					freemsg (*rp_bp);
					ret = FAILURE;
					goto out;
				}
			}

			freemsg (*rp_bp);
			goto getit;
		}  else goto out;

	/*  copy from user on this machine to kernel on other machine	*/
	case DUCOPYIN:
	{
		char *from;
		int n;

		from = (char *)resp->rp_bufptr;
		n = resp->rp_count;
		for( ; n > 0; n -= DATASIZE, from += DATASIZE ){
			nbp = getcbp (sizeof (struct response));
			nresp = (struct response *) PTOMSG(nbp->b_rptr);
			nresp->rp_type = RESP_MSG;
			nresp->rp_opcode = DUCOPYIN;
			nresp->rp_count = (n > DATASIZE) ? DATASIZE : n;
			nresp->rp_errno = 0;
			if (u.u_syscall == DUWRITEI && u.u_segflg == 1)
			      bcopy (from, nresp->rp_data, nresp->rp_count);
			else  {
				if (copyin (from, nresp->rp_data,
				    nresp->rp_count) < 0)  {
					nresp->rp_errno = EFAULT;
					u.u_error = EFAULT;
					nresp->rp_count = 0;
				}
			}

			/* check if write data should be put in the cache */
			if (nresp->rp_errno)
				goto out1;
			if ((u.u_rcstat & U_RCACHE) &&
			  (u.u_syscall == DUWRITE || u.u_syscall == DUWRITEI)) {
				int blkct;
				blkct = ct_to_blkct(nresp->rp_count,sd->sd_offset);
				/*
				 * Check if write data in cache.  If not, get
				 * an empty buffer and add to cache if
				 * it's a whole-block write. 
				 */
				if ((rbp = rget_cache(sd, 
				   off_to_blkoff(sd->sd_offset),blkct))==NULL) {
					rcinfo.cbwrite += blkct;
					if ((nresp->rp_count != RBSIZE) ||
					  ((RBMASK & sd->sd_offset) != 0)) {
						goto out1;
					}
					rbp = rgeteblk(sd, 
						off_to_blkoff (sd->sd_offset));
				}
				if (rbp) {
					rcache_in(rbp, sd, nresp->rp_data, nresp->rp_count);
					rbp = NULL;
				}
				else
					sd->sd_offset += nresp->rp_count;
			}
					
out1:
			if(sndmsg (gift, nbp, sizeof (struct response) - 
				DATASIZE + nresp->rp_count, (rcvd_t)NULL) == FAILURE) {
				freemsg(*rp_bp);
				ret = FAILURE;
				goto out;
			}
			if (u.u_error == EFAULT) {
				freemsg (*rp_bp);
				goto getit;
			}
		}
		freemsg (*rp_bp);
		goto getit;
	}
	} /* end of switch */

out:	u.u_procp->p_flag &= ~SRSIG;	/*clear any remote signal flags*/	
	free_rcvd (rd);
	if (u.u_rcstat & U_RCACHE) {
		/*
		 * Disable-cache message could arrive in middle, 
		 * in which case invalidate the buffers.
		 */
		if (!(sd->sd_stat & SDCACHE))
			rfinval(sd, -1 , 0);
		/*
		 * Error could leave hanging the buffer currently being filled.
		 * If this happens, invalidate contents and release buffer.
		 */
		if (rbp) {
			rbp->b_flags |= B_STALE|B_AGE;
			rbrelse(rbp);
		}
	}
	return (ret);
}
