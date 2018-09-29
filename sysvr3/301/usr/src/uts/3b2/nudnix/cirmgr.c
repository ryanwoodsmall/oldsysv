/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/cirmgr.c	10.26"
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/signal.h"
#include "sys/file.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/var.h"
#include "sys/comm.h"
#include "sys/recover.h"
#include "sys/inline.h"
#include "sys/rdebug.h"
#include "sys/nserve.h"
#include "sys/inode.h"
#include "sys/cirmgr.h"
#include "sys/tihdr.h"
#include "sys/message.h"


extern	rcvd_t rd_recover;	/* special recovery rd */
static struct module_info 	gdpr_info = {0, NULL, 0, INFPSZ, 4*1024, 1024};
static struct module_info 	gdpw_info = {0, NULL, 0, INFPSZ, 0, 0};
extern int 	gdp_rput(), gdp_rsrv(),gdp_wsrv(), nulldev();
static struct qinit 	gdprdata = 
		{ gdp_rput, gdp_rsrv, nulldev, nulldev, nulldev, &gdpr_info, NULL };
static struct qinit 	gdpwdata =
		{ nulldev, gdp_wsrv, nulldev, nulldev, nulldev, &gdpw_info, NULL };
extern int msgflag;

/*
 *	Obtain the queue pointer representing a remote machine.  Normally, 
 *	take the queue associated with fd (it had better be streams-based),
 *	and associate with it the token, for future reference.  If the
 *	fd is -1, then return the queue pointer previously associated
 *	with token.
 *	Will fail if a) the fd is -1, and token doesn't match any of
 *	those stored, or b) fd is OK, but no more entry in the table.
 */

queue_t *
get_circuit (fd,tokenp)
register struct token *tokenp;
{
	queue_t *init_circuit ();
	register struct gdp *tmp;

	for (tmp = gdp; tmp < &gdp[maxgdp] ; tmp++)
	    if (fd >= 0) 
		if(tmp->flag == GDPFREE)
			return(init_circuit(fd,tmp));
		else 
			continue;
	    else if ((tmp->flag & GDPCONNECT) && (tokcmp (&(tmp->token), tokenp))) {
		DUPRINT2(DB_GDP,"getcircuit: ckt exists qp = %x\n",gdp->queue);
		return (tmp->queue);
	    	}

	DUPRINT1(DB_GDP,"getcircuit: circuit doesn't exist\n");
	u.u_error = ECOMM;
	return(NULL);
}

put_circuit (qp)
queue_t *qp;
{
	register struct gdp *gdpp = GDP(qp);

	DUPRINT2(DB_GDP,"put_circuit: qp %x\n", qp);
	if (gdpp->mntcnt) 
		return;
	clean_circuit (qp);
}


gdp_init ()
{
	register struct gdp *tmp;
	register int i;

	DUPRINT1(DB_GDP,"gdp_init: initializing gdp\n");
	for (tmp = gdp, i = 0; i < maxgdp; tmp++) {
		tmp->queue = NULL;
		tmp->sysid = ++i; /* set the local half of sysid --
					the remote half is set when
					the first message arrives */
		tmp->flag = GDPFREE;
		tmp->token.t_id = 0;
		tmp->token.t_uname[0] = '\0';
		tmp->idmap[0] = 0;
		tmp->idmap[1] = 0;
	}
}

gdp_rsrv (qp)
register queue_t *qp;
{
	register mblk_t *msg;
	register struct message *message;
	register long tprim;
	
	while ((msg = getq (qp)) != NULL) {
		tprim = ((union T_primitives *)msg->b_rptr)->type;
		DUPRINT2(DB_GDP,"gdp_rsrv: got tli primitive %d\n",tprim);
		switch(tprim) {
		case T_DATA_IND:
		{
			register mblk_t *smgs;
			register mblk_t *tmgs;
			extern	mblk_t *gdp_pullupmsg();

			tmgs = msg->b_cont;
			if((smgs = gdp_pullupmsg(tmgs)) == NULL) {
				DUPRINT1(DB_GDPERR, "gdp_rsrv: couldn't pullupmsg\n");
				/* put the msg back in queue */
				putbq(qp, msg);
				bufcall(msgdsize(tmgs), BPRI_MED, qenable, qp);
				return;
			}
			freeb(msg);

			/*
			 *	convert RFS headers and data back to local forms
			 */
			rffrcanon(smgs, GDP(qp)->hetero);

			message = (struct message *)smgs->b_rptr;
			/* mark queue from whence message came */
			message->m_queue = (long)qp; 
			arrmsg(smgs);
			break;
		}
		case T_DISCON_IND:
			GDP(qp)->flag = GDPDISCONN;
			msgflag |= DISCONN;
			wakeup(&rd_recover->rd_qslp);
			freemsg(msg);
			break;
		default:
			DUPRINT2(DB_GDP,"gdp_rsrv: received invalid tli primitive %d\n",tprim);
			freemsg(msg);
			break;
		} /*end switch */
	} /* end while */
}

gdp_rput (q, bp)
register queue_t	*q;
register mblk_t *bp;
{
	switch (bp->b_datap->db_type) {
	case M_DATA:
	case M_PROTO:
		putq (q, bp);
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHR) flushq(q, FLUSHALL);
		if (*bp->b_rptr & FLUSHW) {
			*bp->b_rptr &= ~FLUSHR;
			putnext(WR(q), bp);
			break;
		}
		freemsg(bp);
		break;

	case M_ERROR:	
		printf("cirmgr: gdp_rput: M_ERROR q %x\n",q);
		GDP(q)->flag = GDPDISCONN;
		msgflag |= DISCONN;
		wakeup(&rd_recover->rd_qslp);
		freemsg(bp);
		break;
	default:
		printf("cirmgr: gdp_rput: illegal message type %o\n", bp->b_datap->db_type);
		freemsg(bp);
		break;
	}
}


/* attaches the stream described by fd to gdp struct pointed at by gdpp */
static queue_t *
init_circuit (fd, gdpp)
register struct gdp *gdpp;
{
	register queue_t *qp;
	register struct stdata *stp;
	register struct file *fp;
	register int s;

	/* Deny for lots of reasons.... */
	if ( (fd < 0 || fd >= NOFILE) ||
		!(fp = u.u_ofile[fd]) || 
		(u.u_pofile[fd] & FREMOTE) ||
		(fp->f_count != 1) ||
		(fp->f_inode->i_count != 1) ||
		!(stp = fp->f_inode->i_sptr) )
	{
	    DUPRINT4(DB_GDP,"bad fd to init_circuit fd = %x, fp = %x, stp = %x\n", fd, fp, stp );
	    u.u_error = EBADF;
	    return(NULL);
	}


	s = splstr();

	qp = RD(stp->sd_wrq); 		/* steal the stream head's queue's */
	stp->sd_flag |= STPLEX;		/* in case someone else can open late */
	fp->f_count++;			/* for consistency */

	/* point gdp at q */
	gdpp->queue = qp;
	gdpp->mntcnt = 0;
	gdpp->time = 0;

	/* point q at gdp structure  and intialize q */
	qp->q_ptr = WR(qp)->q_ptr = (caddr_t)gdpp;
	setq (qp,&gdprdata,&gdpwdata);
	qp->q_flag |= QWANTR;

	gdpp->file = fp;
	gdpp->flag = GDPCONNECT;
	if(qp->q_first)
		qenable(qp);
	splx(s);

	return (qp);
}

tokclear(gp)
register struct gdp *gp;
{
	gp->token.t_id = 0;
	gp->token.t_uname[0] = '\0';
}

clean_circuit (qp)
register queue_t *qp;
{
	extern struct	qinit strdata, stwdata;
	register struct gdp *gdpp;
	register struct stdata *stp;
	register int s;

	DUPRINT2(DB_GDP,"clean_circuit %x\n",qp);
	gdpp = GDP(qp);
	if (gdpp->flag == GDPFREE)
		return;

	/* restore queues to their rightful owner, the stream head */

	stp = gdpp->file->f_inode->i_sptr;
	s = splstr();
	setq (qp,&strdata,&stwdata);
	qp->q_ptr = WR(qp)->q_ptr = (caddr_t)stp;
	splx(s);

	stp->sd_flag &= ~STPLEX;
	closef(gdpp->file);	/* and close it */
	gdpp->file = NULL;
	gdpp->queue = NULL;
	gdpp->flag = GDPFREE;
	gdpp->sysid = gdpp - gdp + 1;
	gdpp->hetero = 0;
	gdpp->version = 0;
	gdpp->time = 0;
	gdpp->mntcnt = 0;
	tokclear (gdpp);
	if (gdpp->idmap[0])
		freeidmap(gdpp->idmap[0]);
	if (gdpp->idmap[1])
		freeidmap(gdpp->idmap[1]);
	gdpp->idmap[0] = 0;
	gdpp->idmap[1] = 0;
}			

void
kill_gdp()
{
	register struct gdp *tmp;

	DUPRINT1(DB_GDP,"kill_gdp\n");
	for (tmp = gdp; tmp < &gdp[maxgdp] ; tmp++)
	    if (tmp->flag != GDPFREE) 
		clean_circuit (tmp->queue);
}


tokcmp(n1, n2)
struct token *n1, *n2;
{
	register char	*p1=n1->t_uname, *p2=n2->t_uname;
	if (n1->t_id != n2->t_id)
		return(0);
	if (strncmp(p1,p2,MAXDNAME) == 0)
		return(1);
	return(0);
}
gdp_wsrv(qp)
queue_t *qp;
{
	DUPRINT2(DB_GDP,"gdp_wsrv qp %x\n",qp);
	wakeup(qp->q_ptr);
}


/*
 *	Call pullupmsg() to gather all data into one message and aligned.
 *	If pullupmsg() fails, try to use the incoming message buffer to do
 *	an in-place copy to make the message word aligned if alignment is
 *	the reason for doing pullupmsg().
 *	Otherwise, we have to put the message back into stream queue, this may
 *	potentially lead to a deadlock situation.
 */

mblk_t *
gdp_pullupmsg(bp)
register mblk_t	*bp;
{

	/* pullup all data in the message */
	if (pullupmsg(bp, -1)) {
		return(bp);
	}

	/* fail to pullup all data, try to do in place copy */
	if (bp->b_cont || bp->b_datap->db_ref != 1)
		return(NULL);
	DUPRINT2(DB_GDPERR, "gdp_pullupmsg: copy bp %x\n", bp);
	bcopy(bp->b_rptr, bp->b_datap->db_base, bp->b_wptr-bp->b_rptr);
	bp->b_wptr = bp->b_datap->db_base + (bp->b_wptr - bp->b_rptr);
	bp->b_rptr = bp->b_datap->db_base;
	return(bp);
}
