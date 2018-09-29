/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/log.c	10.4"

/*
 * Streams log driver.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strstat.h"
#include "sys/log.h"
#include "sys/strlog.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/inline.h"

static struct module_info logm_info = 
	{LOG_MID, LOG_NAME, LOG_MINPS, LOG_MAXPS, LOG_HIWAT, LOG_LOWAT};

int logopen(), logclose(), logput(), logrsrv();

static struct qinit logrinit = 
		{ NULL, logrsrv, logopen, logclose, NULL, &logm_info, NULL };
static struct qinit logwinit = 
		{ logput, NULL, NULL, NULL, NULL, &logm_info, NULL };

struct streamtab loginfo = { &logrinit, &logwinit, NULL, NULL };

struct log *log_errp;			/* pointer to error logger, used by strlog() */
struct log *log_trcp;			/* pointer to trace logger, used by strlog() */
static mblk_t *log_trcmsg;		/* message containing trace_ids structures */
static struct trace_ids *log_trcids;	/* pointer to base of trace_ids array */
static int log_ntrcid;			/* the number of trace_ids */
static int log_errseq, log_trcseq;	/* logger sequence numbers */




/*
 * Initialization function - called during system initialization to 
 * prepare the id table
 */

loginit()
{
	log_errp = NULL;
	log_trcp = NULL;
}


/*
 * Device open call 
 */

logopen(q, dev, flag, sflag)
queue_t *q;
{
	int i;
	struct log *lp;

	if (sflag != CLONEOPEN) return(OPENFAIL);
	lp = log_log;
	for (i=0; i<log_cnt; i++, lp++) {
		if (!(lp->log_state & LOGOPEN)) break;
	}
	if (i >= log_cnt) return(OPENFAIL);
	lp->log_state = LOGOPEN;
	lp->log_rdq = q;
	lp->log_bcnt = 0;
	q->q_ptr = (caddr_t)lp;
	WR(q)->q_ptr = (caddr_t)lp;
	return(lp - log_log);
}


/*
 * Device close call
 */

logclose(q)
queue_t *q;
{
	struct log *lp;

	if (!q->q_ptr) return;	/* prevents a panic in close race condition */

	lp = (struct log *)(q->q_ptr);
	lp->log_state = 0;
	lp->log_rdq = NULL;
		
	if (lp == log_errp) log_errp = NULL;
	if (lp == log_trcp) {
		log_trcp = NULL;
		freemsg(log_trcmsg);
	}
	flushq(q,1);
	flushq(WR(q), 1);
	q->q_ptr = NULL;
	WR(q)->q_ptr = NULL;
}

/*
 * Write queue put procedure.  
 */

logput(q, bp)
queue_t *q;
mblk_t *bp;
{
	register s;
	register struct iocblk *ioc;
	register struct log *lp;
	struct log_ctl *lcp;
	mblk_t *bp2, *dp2;
	int both;

	switch (bp->b_datap->db_type) {

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) flushq(q, 0);
		if (*bp->b_rptr & FLUSHR) {
			flushq(RD(q), 0);
			*bp->b_rptr &= ~FLUSHW;
			qreply(q, bp);
			return;
		}
		freemsg(bp);
		return;

	case M_IOCTL:
		lp = (struct log *)q->q_ptr;
		ioc = (struct iocblk *)bp->b_rptr;
		switch (ioc->ioc_cmd) {

		case I_TRCLOG:
			if (!log_trcp && bp->b_cont) {
				log_trcp = lp;
				log_trcmsg = bp->b_cont;
				bp->b_cont = NULL;
				log_trcids = (struct trace_ids *)log_trcmsg->b_rptr;
				log_ntrcid = (long)(log_trcmsg->b_wptr - 
						    log_trcmsg->b_rptr) / 
							sizeof(struct trace_ids);
				ioc->ioc_count = 0;
				bp->b_datap->db_type = M_IOCACK;
				qreply(q,bp);
				return;
			}
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;

		case I_ERRLOG:
			if (!log_errp) {
				log_errp = lp;
				bp->b_datap->db_type = M_IOCACK;
			} else
				bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;

		default:
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}

	case M_PROTO:
		if (((bp->b_wptr - bp->b_rptr) != sizeof(struct log_ctl)) ||
		    !bp->b_cont ) {
			freemsg(bp);
			return;
		}
		lcp = (struct log_ctl *)(bp->b_rptr);
		if (lcp->flags & SL_ERROR) {
			log_errseq++;
			if (!log_errp) lcp->flags &= ~SL_ERROR;
		}
		if (lcp->flags & SL_TRACE) {
			log_trcseq++;
			if (!log_trcp || !logtrchk(LOG_MID,
						   (struct log *)(q->q_ptr) - log_log,
						   lcp->level) )
				lcp->flags &= ~SL_TRACE;
		}

		if (!(lcp->flags & (SL_ERROR|SL_TRACE))) {
			freemsg(bp);
			return;
		}

		if ( (lcp->flags & (SL_ERROR|SL_TRACE)) == (SL_ERROR|SL_TRACE) ) both = 1;
		else both = 0;

		lcp->ltime = lbolt;
		lcp->ttime = time;
		lcp->mid = LOG_MID;
		lcp->sid = (struct log *)(q->q_ptr) - log_log;
		if (lcp->flags & SL_TRACE) {
			lcp->seq_no = log_trcseq;
			if (both) {
				if (bp2 = copyb(bp)) {
					if (dp2 = dupb(bp->b_cont)) {
						bp2->b_cont = dp2;
						s = splstr();
						putq(log_trcp->log_rdq, bp2);
						if (log_trcp->log_bcnt >= log_bsz) 
							freemsg(getq(log_trcp->log_rdq));
						else
							log_trcp->log_bcnt++;
						splx(s);
					}
					else freeb(bp2);
				}
			} else {
				s = splstr();
				putq(log_trcp->log_rdq, bp);
				if (log_trcp->log_bcnt >= log_bsz) 
					freemsg(getq(log_trcp->log_rdq));
				else
					log_trcp->log_bcnt++;
				splx(s);
				return;
			}
		}
		if (lcp->flags & SL_ERROR) {
			lcp->seq_no = log_errseq;
			s = splstr();
			putq(log_errp->log_rdq, bp);
			if (log_errp->log_bcnt >= log_bsz) 
				freemsg(getq(log_errp->log_rdq));
			else
				log_errp->log_bcnt++;
			splx(s);
			return;
		}
		return;		

	default:
		bp->b_datap->db_type = M_ERROR;
		bp->b_rptr = bp->b_datap->db_base;
		bp->b_wptr = bp->b_rptr + 1;
		*bp->b_rptr = EIO;
		qreply(q, bp);
		return;
	}
}



/*
 * Kernel logger interface function.  Attempts to construct a log
 * message and send it up the logger stream.  Delivery will not be
 * done if message blocks cannot be allocated or if the logger
 * is not registered.
 */

strlog(mid,sid,level,flags,fmt,arg)
short mid,sid;
char level;
unsigned short flags;
char *fmt;
unsigned arg;
{
	register s;
	register char *dst, *src;
	register i;
	unsigned char both;
	register unsigned *argp;
	mblk_t *cbp, *dbp; 
	mblk_t *cbp2, *dbp2;
	queue_t *qp;
	union log_req *srp;
	struct log_ctl *lcp;

	ASSERT(flags & (SL_ERROR | SL_TRACE));

	if (flags & SL_ERROR) {
		log_errseq++;
		if (!log_errp) flags &= ~SL_ERROR;
	}
	if (flags & SL_TRACE) {
		log_trcseq++;
		if (!log_trcp || !logtrchk(mid,sid,level) )
			flags &= ~SL_TRACE;
	}

	if (!(flags & (SL_ERROR|SL_TRACE))) return;

	if ( (flags & (SL_ERROR|SL_TRACE)) == (SL_ERROR|SL_TRACE) ) both = 1;
	else both = 0;
	
	/*
	 * allocate message blocks for log text, log header, and 
	 * proto control field.
	 */
	if (!(dbp = allocb(LOGMSGSZ, BPRI_HI))) return;
	if (!(cbp = allocb(sizeof(struct log_ctl), BPRI_HI))){
		freeb(dbp);
		return;
	}

	/*
	 * copy log text into text message block.  This consists of a 
	 * format string and NLOGARGS integer arguments.
	*/
	dbp->b_datap->db_type = M_DATA;
	dst = (char *)dbp->b_wptr;
	src = fmt;
	logstrcpy(dst, src);
	/*
	 * dst now points to the null byte at the end of the format string.  
	 * Move the wptr to the first int boundary after dst.
	 */
	dbp->b_wptr = (unsigned char *)logadjust(dst);

	ASSERT((int)(dbp->b_datap->db_lim - dbp->b_wptr) >= NLOGARGS*sizeof(int));
	argp = &arg;
	for (i=0; i<NLOGARGS; i++) {
		*((int *)dbp->b_wptr) = *argp++;
		dbp->b_wptr += sizeof(int);
	}

	/*
	 * set up proto header
	 */
	cbp->b_datap->db_type = M_PROTO;
	cbp->b_cont = dbp;
	cbp->b_wptr += sizeof(struct log_ctl);
	lcp = (struct log_ctl *)cbp->b_rptr;
	lcp->mid = mid;
	lcp->sid = sid;
	lcp->ltime = lbolt;
	lcp->ttime = time;
	lcp->level = level;
	lcp->flags = flags;

	if (flags & SL_TRACE) {
		lcp->seq_no = log_trcseq;
		if (both) {
			if (cbp2 = copyb(cbp)) {
				if (dbp2 = dupb(dbp)) {
					cbp2->b_cont = dbp2;
					s = splstr();
					putq(log_trcp->log_rdq, cbp2);
					if (log_trcp->log_bcnt >= log_bsz) 
						freemsg(getq(log_trcp->log_rdq));
					else
						log_trcp->log_bcnt++;
					splx(s);
				}
				else freeb(cbp2);
			}
		} else {
			s = splstr();
			putq(log_trcp->log_rdq, cbp);
			if (log_trcp->log_bcnt >= log_bsz) 
				freemsg(getq(log_trcp->log_rdq));
			else
				log_trcp->log_bcnt++;
			splx(s);
			return;
		}
	}
	if (flags & SL_ERROR) {
		lcp->seq_no = log_errseq;
		s = splstr();
		putq(log_errp->log_rdq, cbp);
		if (log_errp->log_bcnt >= log_bsz) 
			freemsg(getq(log_errp->log_rdq));
		else
			log_errp->log_bcnt++;
		splx(s);
		return;
	}
}


/*
 * Send a log message up a given log stream.
 */
logrsrv(q)
queue_t *q;
{
	mblk_t *mp;
	struct log *lp;
	register s;

	lp = (struct log *)q->q_ptr;
	s = splstr();
	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			splx(s);
			return;
		}
		lp->log_bcnt--;
		putnext(q, mp);
	}
	splx(s);
}



/*
 * Check mid,sid against list of values requested by the tracer.
 */
logtrchk(mid, sid, level)
register short mid, sid;
register char level;
{
	register struct trace_ids *tid;
	register i;

	ASSERT(log_trcp);

	for (tid = log_trcids, i=0; i < log_ntrcid; tid++, i++) {
		if ((tid->ti_level < level) && (tid->ti_level >= 0)) continue;
		if ((tid->ti_mid != mid) && (tid->ti_mid >= 0)) continue;
		if ((tid->ti_sid != sid) && (tid->ti_sid >= 0)) continue;
		return(1);
	}
	return(0);
}
