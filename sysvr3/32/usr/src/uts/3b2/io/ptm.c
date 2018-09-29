/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/ptm.c	10.6"
/*
 *	         Copyright (c) 1987 AT&T
 *		   All Rights Reserved
 *
 *     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any actual
 *   or intended publication of such source code.
 *
 ================================================================
 =   WARNING!!!! This source is not supported in future source  =
 =   releases.                                                  =
 ================================================================
 */


#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/ptms.h"

#ifdef DBUG
#define DEBUG(a)	 if (ptm_debug) cmn_err( CE_NOTE, a) 
#else
#define DEBUG(a)
#endif 

int ptm_debug = 0;


/*
 *   Master Stream Pipe
 */

static struct module_info ptm_info = {0xdead, "ptm", 0, INFPSZ, 512, 128};
static int ptmopen(), ptmclose(), ptmwput(), ptmrsrv(), ptmwsrv();
static struct qinit ptmrint = {NULL, ptmrsrv, ptmopen, ptmclose,NULL,&ptm_info, NULL};
static struct qinit ptmwint = {ptmwput, ptmwsrv, ptmopen, ptmclose, NULL,&ptm_info, NULL};
struct streamtab ptminfo = {&ptmrint, &ptmwint, NULL, NULL};

/*
 * ptms_tty[] and pt_cnt are defined in master.d file 
 */
extern struct pt_ttys ptms_tty[];	
extern int pt_cnt;

/*
 * Open a minor of the master device. Find an unused entry in the
 * ptms_tty array. Store the write queue pointer and set the
 * pt_state field to (PTMOPEN | PTLOCK).
 */
static int 
ptmopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
        register struct pt_ttys *ptmp;

	DEBUG(("entering ptmopen\n"));
	for(dev=0; dev < pt_cnt; dev++)
		if (!(ptms_tty[dev].pt_state & 
			(PTMOPEN | PTSOPEN | PTLOCK)))
				break;

	if (sflag != CLONEOPEN) {
		DEBUG(("invalid sflag\n"));
		u.u_error = EINVAL;
		return(OPENFAIL);
	}

        if (dev >= pt_cnt) {
		DEBUG(("no more devices left to allocate\n"));
		u.u_error = ENODEV;
                return(OPENFAIL);
	}

        ptmp = &ptms_tty[dev];
	if (ptmp->pts_wrq)
		if ((ptmp->pts_wrq)->q_next) {
			DEBUG(("send hangup to an already existing slave\n"));
			putctl(RD(ptmp->pts_wrq)->q_next, M_HANGUP);
		}
	/*
	 * if process leader with no controlling tty and 
	 * stream is unassigned, assign controlling tty.
	 */
	if (( u.u_procp->p_pid == u.u_procp->p_pgrp)
		&& ( u.u_ttyp == NULL) && ( ptmp->tty == 0)) {
		u.u_ttyp = &ptmp->tty; 
		ptmp->tty = u.u_procp->p_pgrp;
	}

	/*
	 * set up the entries in the pt_ttys structure for this
	 * device.
	 */
	ptmp->pt_state = (PTMOPEN | PTLOCK);
	ptmp->ptm_wrq = WR(rqp);
	ptmp->pts_wrq = NULL;
	ptmp->tty = u.u_procp->p_pgrp;
	ptmp->pt_bufp = NULL;
	WR(rqp)->q_ptr = (char *) ptmp;
	rqp->q_ptr = (char *) ptmp;

	DEBUG(("returning from ptmopen()\n"));
	return(dev);
}


/*
 * Find the address to private data identifying the slave's write queue.
 * Send a hang-up message up the slave's read queue to designate the
 * master/slave pair is tearing down. Uattach the master and slave by 
 * nulling out the write queue fields in the private data structure.  
 * Finally, unlock the master/slave pair and mark the master as closed.
 */
static int 
ptmclose(rqp)
queue_t *rqp;
{
        register struct pt_ttys *ptmp;
        register queue_t *pts_rdq;

	ASSERT(rqp->q_ptr);
		
	DEBUG(("entering ptmclose\n"));
        ptmp = (struct pt_ttys *)rqp->q_ptr;
	if (ptmp->pts_wrq) {
		pts_rdq = RD(ptmp->pts_wrq);
		if(pts_rdq->q_next) {
			DEBUG(("send hangup message to slave\n"));
			putctl(pts_rdq->q_next, M_HANGUP); 
		}
	}
	freemsg(ptmp->pt_bufp);
	ptmp->pt_bufp = NULL;
        ptmp->ptm_wrq = NULL;
	ptmp->pt_state &= ~(PTMOPEN | PTLOCK);
	ptmp->tty = 0;
	rqp->q_ptr = NULL;
	WR(rqp)->q_ptr = NULL;

	DEBUG(("returning from ptmclose\n"));
	return;
}

/*
 * The wput procedure will only handle ioctl and flush messages.
 */
static int 
ptmwput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
        register struct pt_ttys *ptmp;
	struct iocblk *iocp;
	
	DEBUG(("entering ptmwput\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
		
	switch(mp->b_datap->db_type) {
		/*
		 * if write queue request, flush master's write
		 * queue and send FLUSHR up slave side. If read 
		 * queue request, convert to FLUSHW and putctl1().
		 */
		case M_FLUSH:
			DEBUG(("ptm got flush request\n"));
			if (*mp->b_rptr & FLUSHW) {
				DEBUG(("flush ptm write Q\n"));
				flushq(qp, FLUSHDATA);
				if(ptmp->pts_wrq && !(ptmp->pt_state & PTLOCK)) {
					DEBUG(("putctl1 FLUSHR to pts\n"));
					putctl1(RD(ptmp->pts_wrq)->q_next, M_FLUSH, FLUSHR);
				}
			}
			if (*mp->b_rptr & FLUSHR) {
				DEBUG(("got read, putctl1 FLUSHW\n"));
				if(ptmp->pts_wrq && !(ptmp->pt_state & PTLOCK))
					putctl1(RD(ptmp->pts_wrq)->q_next, M_FLUSH, FLUSHW);
			}
			freemsg(mp);
			break;

		case M_IOCTL:
			iocp = (struct iocblk *)mp->b_rptr;
			switch(iocp->ioc_cmd) {
			default:
				 if((ptmp->pt_state  & PTLOCK) || 
				    (ptmp->pts_wrq == NULL)) {
					DEBUG(("got M_IOCTL but no slave\n"));
					mp->b_datap->db_type = M_IOCNAK;
					freemsg(mp->b_cont);
					mp->b_cont = NULL;
					qreply(qp, mp);
					return;
				}
				putq(qp, mp);
				break;
			case ISPTM:
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				DEBUG(("ack the ISPTM\n"));
				qreply(qp, mp);
				break;
			case UNLKPT:
				ptmp->pt_state &= ~PTLOCK;
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				DEBUG(("ack the UNLKPT\n"));
				qreply(qp, mp);
				break;
			}
			break;
		
		/*
		 * send other messages to slave
		 */
		default:
			if((ptmp->pt_state  & PTLOCK) || (ptmp->pts_wrq == NULL)) {
				DEBUG(("got msg. but no slave\n"));
				putctl1(RD(qp)->q_next, M_ERROR,EINVAL);
				freemsg(mp);
				return;
			}
			DEBUG(("put msg on master's write queue\n"));
			putq(qp, mp);
			break;
	}
	DEBUG(("return from ptmwput()\n"));
	return;
}


/*
 * enable the write side of the slave. This triggers the 
 * slave to send any messages queued on its write side to
 * the read side of this master.
 */
static int 
ptmrsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptmp;
	
	DEBUG(("entering ptmrsrv\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
	qenable(ptmp->pts_wrq);
	DEBUG(("leaving ptmrsrv\n"));
}
		


/*
 * If there are messages on this queue that can be sent to 
 * slave, send them via putnext(). Else, if queued messages 
 * cannot be sent, leave them on this queue. If priority 
 * messages on this queue, send them to slave no matter what.
 */
static int 
ptmwsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptmp;
        register queue_t *pts_rdq;
	mblk_t *mp;
	
	DEBUG(("entering ptmwsrv\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
	if (ptmp->pts_wrq)
		pts_rdq = RD(ptmp->pts_wrq);
		
	if((ptmp->pt_state  & PTLOCK) || (pts_rdq == NULL)) {
		DEBUG(("in master write srv proc but no slave\n"));
		putctl1(RD(qp)->q_next, M_ERROR, EINVAL);
		return;
	}
	/*
	 * while there are messages on this write queue...
	 */
	while ((mp = getq(qp)) != NULL) {
		/*
		 * if don't have control message and cannot put
		 * msg. on slave's read queue, put it back on 
		 * this queue.
		 */
		if (mp->b_datap->db_type <= QPCTL && 
				!canput(pts_rdq->q_next)) {
			DEBUG(("put msg. back on queue\n"));
			putbq(qp, mp);
			break;
		}
		/*
		 * else send the message up slave's stream
		 */
		DEBUG(("send message to slave\n"));
		putnext(pts_rdq, mp);
	}
	DEBUG(("leaving ptmwsrv\n"));
}

