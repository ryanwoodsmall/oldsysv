/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rsc.c	10.19"

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

extern	rcvd_t	cr_rcvd ();
extern	mblk_t	*dupmsg();
extern	mblk_t	*reusebuf();


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
	int	size, i;		/* ignored 				*/
	int	nackcnt = 0;	/* GDP NACK count 			*/

	if ((rd = cr_rcvd (FILE_QSIZE, SPECIFIC)) == NULL) {
		u.u_error = ENOMEM;
		freemsg (rq_bp);
		u.u_procp->p_flag &= ~SRSIG;	/*clear any remote signal flags*/	
		return (FAILURE);
	}

	/* keep track of sd we're going out on (client side only) */
	rd->rd_inode = (struct inode *) sd;
	u.u_procp->p_minwd = (struct rcvd *) sd;	/* for remote signal */
sendit:	
	/* duplicate the message before sending out */
	nbp = dupmsg(rq_bp);
	if (nbp == NULL) {
		u.u_error = ENOMEM;
		ret = FAILURE;
		goto out;
	}
	if ((ret = sndmsg (sd, nbp, req_size, rd)) != SUCCESS)
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
	/* server's queue was full, try again */
	case DUGDPNACK:
		freemsg (*rp_bp);
		nackcnt++;
		DUPRINT2(DB_RSC, "rsc: GDP nackcnt %x\n", nackcnt);
		/* request message already in canonical format due to
		   previous transmission,
		   we have to convert them back to local format
		   before sending them again */
		rffrcanon(rq_bp, GDP(sd->sd_queue)->hetero);

		if(u.u_procp->p_flag & SRSIG){	/*if signal msg already sent*/
			smsg = (struct message *)rq_bp->b_rptr;
			smsg->m_stat |= SIGNAL;				/*set signal bit*/
		}	
		goto sendit;

	/* server rec'd request. response coming */
	case DUGDPACK:
		freemsg (*rp_bp);
		freemsg (rq_bp);
		unclean = FALSE;
		goto getit;

	/* move data from kernel on remote machine to user on this machine */
	case DUREAD:
	case DUREADI:
		if(resp->rp_subyte)
			break;
	case DUCOPYOUT:
		if (u.u_error == 0 || resp->rp_errno == 0){
			if (u.u_syscall == DUREADI && u.u_segflg == 1)
				bcopy (resp->rp_data, resp->rp_bufptr, resp->rp_count);
			else
				if (copyout (resp->rp_data,resp->rp_bufptr, resp->rp_count))
					u.u_error = EFAULT;
		}
		if (resp->rp_opcode == DUCOPYOUT)  {
			/*  check if a response should be	
			 *  send back so that the
			 *  other end can do more copyout
			 */
			if (resp->rp_copysync) {
				nbp = reusebuf(rq_bp, sizeof(struct response)-DATASIZE);
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
			nbp = reusebuf (rq_bp, sizeof (struct response));
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
					nresp->rp_count = 0;
				}
			}
			if(sndmsg (gift, nbp, sizeof (struct response) - 
				DATASIZE + nresp->rp_count, (rcvd_t)NULL) == FAILURE) {
				freemsg(*rp_bp);
				ret = FAILURE;
				goto out;
			}
		}
		freemsg (*rp_bp);
		goto getit;
	}
	} /* end of switch */

out:	if (unclean)
		freemsg (rq_bp);
	u.u_procp->p_flag &= ~SRSIG;	/*clear any remote signal flags*/	
	free_rcvd (rd);
	return (ret);
}
