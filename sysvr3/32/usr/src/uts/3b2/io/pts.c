/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/pts.c	10.9"
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
#define DEBUG(a)	 if (pts_debug) cmn_err( CE_NOTE, a) 
#else
#define DEBUG(a)
#endif 

int pts_debug = 0;


/*
 *   Slave Stream Pipe
 */

static struct module_info pts_info = {0xface, "pts", 0, INFPSZ, 512, 128};
static int ptsopen(), ptsclose(), ptswput(), ptsrsrv(), ptswsrv();
static struct qinit ptsrint = {NULL, ptsrsrv, ptsopen, ptsclose,NULL,&pts_info, NULL};
static struct qinit ptswint = {ptswput, ptswsrv, ptsopen, ptsclose, NULL,&pts_info, NULL};
struct streamtab ptsinfo = {&ptsrint, &ptswint, NULL, NULL};

/*
 * ptms_tty[] and pt_cnt are defined in slave.d file 
 */
extern struct pt_ttys ptms_tty[];	
extern int pt_cnt;

/*
 * Open the master device. Reject a clone open and do not allow the
 * driver to be pushed. If the slave/master pair is locked or if
 * the slave is not open, return OPENFAIL. If cannot allocate zero
 * length data buffer, fail open.
 * Upon success, store the write queue pointer in private data and
 * set the PTSOPEN bit in the sflag field.
 */
static int 
ptsopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
        register struct pt_ttys *ptsp;
	register mblk_t *mp;

	DEBUG(("entering ptsopen\n"));

        dev = minor(dev);
        if (sflag) {
		DEBUG(("sflag is set\n"));
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
        if ((dev < 0) || (dev > pt_cnt)) {
		DEBUG(("invalid minor number\n"));
		u.u_error = ENODEV;
                return(OPENFAIL);
	}
        ptsp = &ptms_tty[dev];
        if ((ptsp->pt_state & PTLOCK) || !(ptsp->pt_state & PTMOPEN)) {
		DEBUG(("master is locked or slave is closed\n"));
		u.u_error = EACCES;
		return(OPENFAIL);
	}
	/* 
	 * if already, open simply return...
	 */
	if (ptsp->pt_state & PTSOPEN) {
		DEBUG(("master already open\n"));
		return(dev);
	}
	if (!(mp = allocb(0, BPRI_MED))) {
		DEBUG(("could not allocb(0, pri)\n"));
		u.u_error = EAGAIN;
		return(OPENFAIL);
	}
	ptsp->pt_bufp = mp;
	ptsp->pts_wrq = WR(rqp);
	WR(rqp)->q_ptr = (char *) ptsp;
	rqp->q_ptr = (char *) ptsp;
	ptsp->pt_state |= PTSOPEN;
	/*
	 * if process leader with no controlling tty and 
	 * stream is unasigned, assign controlling tty.
	 */
	if (( u.u_procp->p_pid == u.u_procp->p_pgrp)
		&& ( u.u_ttyp == NULL) && ( ptsp->tty == 0)) {
		u.u_ttyp = &ptsp->tty; 
		ptsp->tty = u.u_procp->p_pgrp;
	}

	DEBUG(("returning from ptsopen\n"));
	return(dev);
}



/*
 * Find the address to private data identifying the slave's write 
 * queue. Send a 0-length msg up the slave's read queue to designate 
 * the master is closing. Uattach the master from the slave by nulling 
 * out master's write queue field in private data.
 */
static int 
ptsclose(rqp)
queue_t *rqp;
{
        register struct pt_ttys *ptsp;
	register mblk_t *mp;

	DEBUG(("entering ptsclose\n"));
	/*
	 * if no private data...
	 */
	if(!rqp->q_ptr)
		return;

        ptsp = (struct pt_ttys *)rqp->q_ptr;
	if((ptsp->ptm_wrq) && (ptsp->pt_bufp)) {
		DEBUG(("putnext() a zero length message\n"));
		putnext(RD(ptsp->ptm_wrq),ptsp->pt_bufp);
	} else
		if(ptsp->pt_bufp)
			freemsg(ptsp->pt_bufp);
	ptsp->pt_bufp = NULL;
        ptsp->pts_wrq = NULL;
	ptsp->pt_state &= ~PTSOPEN;
	ptsp->tty = 0;
	rqp->q_ptr = NULL;
	WR(rqp)->q_ptr = NULL;

	DEBUG(("returning from ptsclose\n"));
	return;
}


/*
 * The wput procedure will only handle flush messages.
 */
static int 
ptswput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
        register struct pt_ttys *ptsp;
	
	DEBUG(("entering ptswput\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DEBUG(("in write put proc but no master\n"));
		freemsg(mp);
		return;
	}
	switch(mp->b_datap->db_type) {
		/*
		 * if write queue request, flush slave's write
		 * queue and send FLUSHR to ptm. If read queue
		 * request, send FLUSHR to ptm.
		 */
		case M_FLUSH:
			DEBUG(("pts got flush request\n"));
			if (*mp->b_rptr & FLUSHW) {
				DEBUG(("flush pts write Q\n"));
				flushq(qp, FLUSHDATA);
				DEBUG(("putctl1 FLUSHR to ptm\n"));
				putctl1(RD(ptsp->ptm_wrq)->q_next, M_FLUSH, FLUSHR);
			}
			if (*mp->b_rptr & FLUSHR) {
				DEBUG(("putctl1 FLUSHW to ptm\n"));
				putctl1(RD(ptsp->ptm_wrq)->q_next, M_FLUSH, FLUSHW);
			}
			freemsg(mp);
			break;
		/*
		 * send other messages to master
		 */
		default:
			DEBUG(("put msg on slave's write queue\n"));
			putq(qp, mp);
			break;
	}
	DEBUG(("return from ptswput()\n"));
	return;
}


/*
 * enable the write side of the master. This triggers the 
 * master to send any messages queued on its write side to
 * the read side of this slave.
 */
static int 
ptsrsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptsp;
	
	DEBUG(("entering ptsrsrv\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DEBUG(("in read srv proc but no master\n"));
		return;
	}
	qenable(ptsp->ptm_wrq);
	DEBUG(("leaving ptsrsrv\n"));
}
		


/*
 * If there are messages on this queue that can be sent to 
 * master, send them via putnext(). Else, if queued messages 
 * cannot be sent, leave them on this queue. If priority 
 * messages on this queue, send them to master no matter what.
 */
static int 
ptswsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptsp;
        register queue_t *ptm_rdq;
	mblk_t *mp;
	
	DEBUG(("entering ptswsrv\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DEBUG(("in write srv proc but no master\n"));
		return;
	}
	ptm_rdq = RD(ptsp->ptm_wrq);
	/*
	 * while there are messages on this write queue...
	 */
	while ((mp = getq(qp)) != NULL) {
		/*
		 * if don't have control message and cannot put
		 * msg. on master's read queue, put it back on
		 * this queue.
		 */
		if (mp->b_datap->db_type<=QPCTL && 
					!canput(ptm_rdq->q_next)) {
			DEBUG(("put msg. back on Q\n"));
			putbq(qp, mp);
			break;
		}
		/*
		 * else send the message up master's stream
		 */
		DEBUG(("send message to master\n"));
		putnext(ptm_rdq, mp);
	}
	DEBUG(("leaving ptswsrv\n"));
}

