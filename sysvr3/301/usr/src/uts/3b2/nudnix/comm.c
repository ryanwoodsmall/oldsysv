/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/comm.c	10.36"
/*  These are the communications routines called from the UNIX kernel.
 *  They deal with send and receive descriptors, and with sending
 *  and receiving messages.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/region.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/stream.h"
#include "sys/message.h"
#include "sys/comm.h"
#include "sys/mount.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/debug.h"
#include "sys/rdebug.h"

#define p_minwdlock	p_trlock

/*
 *  Don't let RFS start unless we have enough resources:
 *	SNDD: one for rfdaemon, one for mount, one for request
 *	RCVD: mount, signals, rfdaemon, one to do something
 *	MINGDP: at least one circuit
 */

#define MINSNDD		3
#define MINRCVD		4
#define MINGDP		1

/*  Global variables used by this routine.  */

int	rdfree;			/*  number of free receive descriptors	*/
int	sdfree;			/*  number of free send descriptors	*/
sndd_t	sdfreelist;		/*  free send desc link list		*/
rcvd_t	rdfreelist;		/*  free recv desc link list		*/
rcvd_t	sigrd;			/*  rd for signals */
rcvd_t	cfrd;			/*  rd for mounts */
static ushort connid;
int	sdlowmark;		/* sd free count low mark */
int	serverslp = 0;		/* server sleep for buffer flag */

extern rcvd_t rd_recover; 	/* recovery receive descriptor */

/*  
 *	create a send descriptor.
 */
sndd_t
cr_sndd ()
{
	register sndd_t	retsndd;
	register int	s;
	register int	i;
	extern   int	nservers;

	s = splrf();
	/* Make sure every server can get a send-descriptor. */
	if (((retsndd = sdfreelist) == NULL) ||
	    (!server() && (sdfree <= (maxserve - nservers)))) {
		DUPRINT1(DB_GDPERR, "cr_sndd: not enough sndds\n");
		splx (s);
		return (NULL);
	}

	/* Got a free sndd. */
	DUPRINT2(DB_SD_RD, "cr_sndd: sd %x\n",retsndd);
	sdfreelist = retsndd->sd_next;
	sdfree--;		
	/* flush nacker queue if sd free count is low */
	if (sdfree < sdlowmark) {
		DUPRINT2(DB_GDPERR, "cr_sndd: send desc free count %x is low\n", sdfree);
		for (i = 0; i < nrcvd; i++) {
			if (rcvd[i].rd_stat == RDUNUSED)
				continue;
			nackflush(&rcvd[i], -1);
		}
	}
	retsndd->sd_refcnt = 1;
	retsndd->sd_queue = NULL;
	retsndd->sd_copycnt = 0;
	retsndd->sd_connid = 0;
	retsndd->sd_sindex = 0;
	retsndd->sd_stat = SDUSED;
	splx(s);
	return (retsndd);
}


set_sndd (sd, queue, index, connid)
register sndd_t sd;
int connid;
queue_t *queue;
index_t index;
{
	DUPRINT5(DB_SD_RD, "set_sndd: sndd %x queue %x index %x proc %x\n",
		 sd, queue, index,u.u_procp);
	ASSERT(sd->sd_stat != SDUNUSED);
	sd->sd_queue = queue;
	sd->sd_sindex = index;
	sd->sd_connid = connid;
}

free_sndd (sd)
sndd_t sd;
{
	int s;
	DUPRINT3(DB_SD_RD, "free_sndd: sd %x, proc %x \n",sd,u.u_procp);
	ASSERT(sd->sd_stat != SDUNUSED);
	s = splrf();
	sd->sd_next = sdfreelist;
	sdfreelist = sd;
	sd->sd_stat = SDUNUSED;
	sd->sd_connid = 0;
	sdfree++;
	splx(s);
}

del_sndd (sd)
sndd_t	sd;
{

	DUPRINT2(DB_SD_RD, "del_sndd: sd %x\n", sd);
	ASSERT(sd->sd_stat != SDUNUSED);
	if(sd->sd_refcnt > 1) 
		sd->sd_refcnt--;
	else {
		mblk_t	*in_bp;
		mblk_t	*out_bp;
		struct	request	*req;

		while ((out_bp = alocbuf (sizeof (struct request) - DATASIZE, BPRI_MED)) == NULL) {
			u.u_procp->p_sig = 0;
		}
		req = (struct request *) PTOMSG(out_bp->b_rptr);
		req->rq_type = REQ_MSG;
		req->rq_sysid = get_sysid(sd); 
		req->rq_uid = u.u_uid;
		req->rq_gid = u.u_gid;
		req->rq_opcode = DUIPUT;
		req->rq_mntindx = sd->sd_mntindx;
		if (rsc (sd, out_bp, sizeof (struct request) - DATASIZE,
		        	&in_bp, (sndd_t)NULL) == FAILURE) {
			if (u.u_error == ENOLINK) {
				DUPRINT1 (DB_SD_RD, 
				"del_sndd succeeds because link is gone\n");
				u.u_error = 0;
			} else {
				DUPRINT2 (DB_SD_RD, 
				"del_sndd: rsc errno %d on CLOSE \n",u.u_error);
			}
		} else {
			freemsg (in_bp);
		}
done:
		free_sndd(sd);
	}
	return;
}

/*	send a message.
 *	set up all the proper fields in the message.  Fill in the gift
 *	field in the message if there is a gift.
 */
sndmsg (sd, bp, bytes, gift)
sndd_t	sd;		/*  which send descriptor to send on		*/
mblk_t	*bp;		/*  message pointer				*/
int	bytes;		/*  how much of the buffer to send		*/
rcvd_t	gift;		/*  gift, if there is one			*/
{
	register struct	message	*msg;
	register mblk_t *tli_msg;
	register int s;

	DUPRINT4(DB_COMM,"sndmsg: sd %x bp %x gift %x\n", sd, bp, gift);
	ASSERT(sd && sd->sd_stat != SDUNUSED);
	if (sd->sd_stat & SDLINKDOWN) {
		DUPRINT1(DB_RECOVER, "sndmsg: trying to send over dead link\n");
		freemsg (bp);
		u.u_error = ENOLINK;
		return (FAILURE);
	}
	ASSERT(sd->sd_queue);
	s = splrf();
	while(!canput(WR(sd->sd_queue)->q_next)) {
		struct gdp *tgdp;
		DUPRINT2(DB_GDPERR, "sndmsg: queue full on sd %x\n", sd);
		tgdp = (struct gdp *)sd->sd_queue->q_ptr;
		sleep(tgdp, PZERO-1);
		if (sd->sd_stat & SDLINKDOWN) {
			DUPRINT1(DB_RECOVER, "sndmsg: trying to send over dead link\n");
			freemsg (bp);
			u.u_error = ENOLINK;
			splx(s);
			return (FAILURE);
		}
	}

	splx(s);
	msg = (struct message *) bp->b_rptr;
	((struct request *) PTOMSG(bp->b_rptr))->rq_pid =
		u.u_procp->p_pid;	/* store the pid for signals */

	bp->b_wptr = bp->b_rptr + sizeof(struct message) + bytes;
	msg->m_dest = sd->sd_sindex;
	msg->m_connid = sd->sd_connid;
	msg->m_cmd = 0;	/* not used */
	if (gift)  {
		msg->m_stat |= GIFT;
		msg->m_gindex = rdtoinx (gift);
		msg->m_gconnid = gift->rd_connid;
		ASSERT(gift->rd_stat != RDUNUSED);
		/* Keep track of who gets gift. */
		if (gift->rd_qtype & SPECIFIC) 
			(struct queue *) gift->rd_user_list = sd->sd_queue;
		/* (Keep track of GENERAL RDs in make_gift.) */
	}
	msg->m_size = bytes + sizeof (struct message);

	/*
	 *	convert RFS headers and data to canonical forms
	 */
	rftocanon(bp, GDP(sd->sd_queue)->hetero);

	putnext( WR(sd->sd_queue), bp);
	if (qready()) runqueues();
	return (SUCCESS);
}


/*  
 *	allocate a send buffer.
 */

mblk_t *
alocbuf (size, bpri)
int	size;
int	bpri;		/* buffer allocation priority */
{
	register mblk_t	*mbp;
	register mblk_t	*nbp;
	int	s;
	extern	setrun();
	extern	mblk_t *server_bp;
	extern	wake_serverbp();

	ASSERT (size <= MSGBUFSIZE);
	while ((mbp = allocb (size + sizeof (struct message), bpri)) == NULL) {
		if (server()) {
			/* server process will use its own stream buffer */
			for (mbp = server_bp; mbp; mbp = mbp->b_next) {
				if (mbp->b_datap->db_ref == 1) {
					DUPRINT2(DB_GDPERR, "alocbuf: use server buffer %x\n", mbp);
					if ((nbp = dupmsg(mbp)) == NULL)
						break;
					nbp->b_rptr = nbp->b_datap->db_base;
					nbp->b_wptr = nbp->b_datap->db_base + sizeof(struct message);
					((struct message *)nbp->b_rptr)->m_stat = 0;
					return (nbp);
				}
			}
			DUPRINT1(DB_GDPERR, "alocbuf: fail to use server buffer\n");
			s = spl5();
			if (serverslp == 0) {
				serverslp++;
				timeout(wake_serverbp, 0, 100); /* sleep for one second */
			}
			splx(s);
			sleep((caddr_t)&server_bp, PZERO);
			continue;
		}

		DUPRINT1(DB_GDPERR, "alocbuf: allocb fail, using bufcall()\n");
		/* wait for buffer to become available */
		if (bufcall (size + sizeof(struct message), bpri, setrun, u.u_procp) == 0) {
			DUPRINT1(DB_GDPERR, "alocbuf: bufcall fail\n");
			return(NULL);
		}
		if (sleep((caddr_t)&(u.u_procp->p_flag), PREMOTE|PCATCH) != 0) {
			/* wake up due to signal */
			strunbcall(size + sizeof(struct message), bpri, u.u_procp);
			DUPRINT1(DB_GDPERR, "alocbuf: wake up by signal\n");
			return(NULL);
		}
		strunbcall(size + sizeof(struct message), bpri, u.u_procp);
	}
	mbp->b_wptr += sizeof(struct message);
	((struct message *)mbp->b_rptr)->m_stat = 0;
	return (mbp);
}


wake_serverbp()
{
	serverslp = 0;
	wakeup((caddr_t)&server_bp);
}



/*
 *	Reuse incoming message buffer for response if the size
 *	is large enough for the response.
 *	Otherwise, allocate a buffer big enough for the response.
 *	This will prevent possible deadlock when server fails to get a
 *	buffer for sending response.
 */

mblk_t *
reusebuf (bp, size)
mblk_t	*bp;
int	size;
{
	mblk_t	*nbp;

	if (size <= (bp->b_datap->db_lim - bp->b_datap->db_base) &&
	    bp->b_datap->db_ref == 1) {
		/* reuse the incoming buffer */
		nbp = dupb(bp);
		if (nbp != NULL) {
			nbp->b_rptr = nbp->b_datap->db_base;
			nbp->b_wptr = nbp->b_datap->db_base + sizeof(struct message);
			((struct message *)nbp->b_rptr)->m_stat = 0;
			return (nbp);
		}
	}

	/* fail to reuse the buffer */
	/* allocate a buffer that is big enough */
	while ((nbp = alocbuf (size, BPRI_MED)) == NULL) {
		u.u_procp->p_sig = 0;
	}
	return (nbp);
}



/*  
 *	Create a receive descriptor.
 */

rcvd_t
cr_rcvd (qsize, type)
int	qsize;		/* max no. messages for this rcvd */
char	type;
{
	register rcvd_t	retrcvd;	/*  return value of the function  */

	if ((retrcvd = rdfreelist) != NULL) {
		rdfreelist = retrcvd->rd_next;
		rdfree--;
		retrcvd->rd_qsize = qsize;
		retrcvd->rd_max_serv = minserve;
		retrcvd->rd_qcnt = 0;
		retrcvd->rd_inode = NULL;
		retrcvd->rd_refcnt = 1;
		retrcvd->rd_act_cnt = 0;
		retrcvd->rd_qtype = type;
		retrcvd->rd_sdnack = NULL;
		retrcvd->rd_user_list = NULL;
		retrcvd->rd_connid = connid++;
		retrcvd->rd_rcvdq.qc_head = NULL;
		retrcvd->rd_stat = RDUSED;
		initsema(&retrcvd->rd_qslp, 0);
		DUPRINT3(DB_SD_RD,"cr_rcvd: rcvd %x index %x\n",
			retrcvd-rcvd, retrcvd);
	}
	else
		DUPRINT1(DB_SD_RD, "cr_rcvd: out of rcvd\n");
	return (retrcvd);
}

/*  
 *	Decrement the count on a receive descriptor.
 *	If count goes to zero, free it and clean text.
 */

del_rcvd (rd, qp)
register rcvd_t	rd;
queue_t *qp;
{
	struct inode *ip;

	DUPRINT4(DB_SD_RD, "del_rcvd: rd %x count %x connid %x\n",
		rd, rd->rd_refcnt,rd->rd_connid);
	ASSERT(rd);
	ASSERT(rd->rd_stat != RDUNUSED);
	if (qp)
		del_rduser(rd, qp);
	if (--rd->rd_refcnt)
		return (FALSE);

	if ((ip = rd->rd_inode) != NULL) {
		if (rd->rd_qtype & RDTEXT && ip->i_count != 1)
			clean_text (ip);
		ip->i_rcvd = NULL;	
	}

	if (server())
		if (u.u_procp->p_minwd == rd)
			u.u_procp->p_minwd = NULL;
	free_rcvd (rd);
	return (TRUE);
}

/*
 *	Return a receive descriptor to the freelist.
 */

free_rcvd (rd)
register rcvd_t	rd;
{
	DUPRINT2(DB_SD_RD, "free_rcvd: rd %x\n", rd);
	ASSERT (rd && rd->rd_stat != RDUNUSED);
	rd->rd_stat = RDUNUSED;
	rdfree++;
	rd->rd_user_list = NULL;
	rd->rd_next = rdfreelist;
	rdfreelist = rd;
}

/*  
 *	dequeue a message off of the receive queue.
 *	Construct a gift from the message if there is one.
 *	The error recovery for no more send descriptors may cause
 *	this routine to sleep waiting for one.
 *	Returns RCVQEMP or SUCCESS.
 */

dequeue (rd, bufp, sd, size)
register rcvd_t	rd;
mblk_t	**bufp;		/*  output - the address of the message buffer	*/
sndd_t	sd;		/*  output - the gift, if there was one		*/
int	*size;		/*  output - how many bytes there are		*/
{
	struct	message	*msg;
	mblk_t	*bp;
	mblk_t	*deque();
	int	s;

	ASSERT(rd);
	/* need to raise priority because arrmsg() calls enque()
	   and add_to_msgs_list() at stream priority */
	s = splrf();
	if ((bp = deque (&rd->rd_rcvdq)) == NULL) {
		splx(s);
		DUPRINT3(DB_COMM, "dequeue: que %x empty,act_cnt=%d\n", rd,rd->rd_act_cnt);
		return (RCVQEMP);
	}
	ASSERT(rd->rd_qcnt > 0);
	DUPRINT5(DB_COMM, "dequeue: rd_act_cnt=%d que=%x, msg %x qcnt %x\n", rd->rd_act_cnt,rd,bp,rd->rd_qcnt);
	rd->rd_qcnt--;
	
	/* send back GDP NACKs if needed because queue is not full now */
	if (rd->rd_sdnack != NULL) {
		nackflush(rd, rd->rd_qsize - rd->rd_qcnt);
	}
	
	msg = (struct message *)bp->b_rptr;
	if ((rcvdemp(rd)) && (rd->rd_qtype & GENERAL)) 
		rm_msgs_list(rd);
	splx(s);

	*bufp = bp;
	*size = msg->m_size - sizeof (struct message);
	if (msg->m_stat & GIFT) {
		struct	request	*msg_in;

		ASSERT(sd);
		msg_in = (struct request *) PTOMSG (msg);
		set_sndd (sd, (queue_t *)msg->m_queue, msg->m_gindex, msg->m_gconnid);
		sd->sd_mntindx = msg_in->rq_mntindx;
	}
	return (SUCCESS);
}

/*	Initialize the communications data structures.  */

comminit ()
{
	extern rcvd_t sigrd;
	sndd_t tmp;
	rcvd_t rd;
	
	if (nsndd < MINSNDD || nrcvd < MINRCVD || maxgdp < MINGDP)
		return(FAILURE);
	if (nsndd <= maxserve) {
		maxserve = (nsndd - 1);
		printf ("maxserve changed to %d - not enough send descriptors\n",
			maxserve);
	}
	if (maxserve < minserve) {
		minserve = maxserve;
		printf ("minserve changed to %d  (maxserve) \n", minserve);
	}
	for (tmp = sndd ; tmp  < &sndd[nsndd]; tmp++) {
		tmp->sd_stat = SDUNUSED;
		tmp->sd_next = tmp + 1;
	}
	sndd[nsndd - 1].sd_next = NULL;
	sdfreelist = sndd;
	if ((sdlowmark = nsndd/10) == 0)
		sdlowmark = 1;
	
	for (rd = rcvd; rd < &rcvd[nrcvd]; rd++) {
		rd->rd_stat = RDUNUSED;
		rd->rd_connid = 0;
		rd->rd_next = rd + 1;
	}
	rcvd[nrcvd - 1].rd_next = NULL;
	rdfreelist = rcvd;
	rdfree = nrcvd;
	sdfree = nsndd;

	/* create well-known RDs */
	cfrd = cr_rcvd (NSQSIZE, GENERAL);	/* chow fun */
	sigrd = cr_rcvd (SIGQSIZE, GENERAL);	/* signals */
	sigrd->rd_max_serv = maxserve;	/*max server count*/
	rd_recover = cr_rcvd (NSQSIZE, SPECIFIC);  /* recovery */

	connid = 0;
	return(SUCCESS);
}

/*	De-initialize the communications data structures.  */

commdinit()
{
	DUPRINT1 (DB_RFSTART, "commdinit \n");
	del_rcvd (cfrd, NULL);
	del_rcvd (sigrd, NULL);
	del_rcvd (rd_recover, NULL);
}

arrmsg (bp)
mblk_t	*bp;
{
	register struct message *msgp = (struct message *)bp->b_rptr;
	register rcvd_t	rd;
	sndd_t	sd;
	struct proc *found;
	extern struct proc *get_free_proc();
	extern void add_to_msgs_list ();
	extern rcvd_t sigrd;
	
	/*
	 *	put the message in the right receive queue
	 */
	ASSERT (msgp->m_dest >= 0 && msgp->m_dest < nrcvd);
	rd = inxtord (msgp->m_dest);
	DUPRINT3(DB_COMM, "arrmsg: msg %x for rd %x\n", bp, rd);

	/*
	ASSERT (rd->rd_connid == msgp->m_connid);
	*/
	
	if (msgp->m_stat & SIGNAL) {
		if(rd->rd_stat == RDUNUSED){
			freemsg(bp);
		DUPRINT2(DB_SIGNAL,"arrmsg:SIG for bad rd %x ignored\n",rd);
			return;
		}
		DUPRINT3(DB_SIGNAL,"arrmsg:m_stat == SIGNAL rd %x sigrd %x\n",rd,sigrd);
		rd = sigrd;
		ASSERT (rd->rd_stat != RDUNUSED);
	}
/* Take nacker queue out
	else {
		ASSERT (rd->rd_stat != RDUNUSED);
		if (rd->rd_qcnt >= rd->rd_qsize) {
			register struct request *msg_in;
			DUPRINT2(DB_RSC, "arrmsg: rd %x queue is full\n", msgp->m_dest);
			ASSERT (msgp->m_stat & GIFT);
			if ((sd = sdfreelist) == NULL) {
				printf("arrmsg: fail to create sd for sending nack, remote syscall may hang\n");
				freemsg(bp);
				return;
			}
			msg_in = (struct request *) PTOMSG(msgp);
			sdfreelist = sd->sd_next;
			sdfree--;		
			sd->sd_refcnt = 1;
			sd->sd_copycnt = 0;
			sd->sd_stat = SDUSED;
			set_sndd (sd, (queue_t *)msgp->m_queue, msgp->m_gindex, msgp->m_gconnid);
			sd->sd_mntindx = msg_in->rq_mntindx;
			DUPRINT2(DB_RSC, "arrmsg: create sd %x for nack\n", sd);
			sd->sd_next = rd->rd_sdnack;
			rd->rd_sdnack = sd;
			freemsg(bp);
			return;
		}
	}
	*/

	enque (&rd->rd_rcvdq, bp);
	rd->rd_qcnt++;
	if (rd->rd_qtype & SPECIFIC)
		cvsema(&rd->rd_qslp);
	else {
		/*dispatch the first free process*/
		if (found = get_free_proc() ) 
			vsema(&found->p_minwdlock, 0);
		add_to_msgs_list (rd); 
	}	
}

/*
 *	nackflush() -- To send GDP NACKS 
 *		rd -- receive descriptor to send back nack
 *		cnt -- number of nacks to be sent back
 *		       -1 means send all nacks back
 */

nackflush(rd, cnt)
register rcvd_t	rd;
register int	cnt;
{
	register mblk_t	*bp;
	register struct	request * req;
	register sndd_t	sd;
	register sndd_t	tmpsd;
	register int	s;

	s = splrf();
	sd = rd->rd_sdnack;
	while (sd != NULL && cnt--) {
		tmpsd = sd;
		rd->rd_sdnack = sd->sd_next;
		sd->sd_srvproc = u.u_procp;
		while ((bp = alocbuf (sizeof (struct request) - DATASIZE, BPRI_MED)) == NULL) {
			u.u_procp->p_sig = 0;
		}
		req = (struct request *) PTOMSG(bp->b_rptr);
		req->rq_type = REQ_MSG;
		req->rq_opcode = DUGDPNACK;
		if (sndmsg (sd, bp, sizeof (struct request) - DATASIZE,(rcvd_t)NULL) == FAILURE) { 
			register int mntindx;
			mntindx = sd->sd_mntindx;
			if (--srmount[mntindx].sr_slpcnt == 0)
				wakeup ( &srmount[mntindx]);
			DUPRINT2(DB_GDPERR, "nackflush: can not send message sd %x\n",sd);
		}
		DUPRINT3(DB_RSC, "nackflush: send NACK for rd %x sd %x\n", rd, sd);
		free_sndd(tmpsd);
		sd = rd->rd_sdnack;
	}
	splx(s);
}


clean_text (ip)
inode_t *ip;
{
	register reg_t		*rp;

	rlstlock();
	for (rp = ractive.r_forw ; rp != &ractive ; rp = rp->r_forw) {
		if (rp->r_type != RT_STEXT)
			continue;
		if (ip == rp->r_iptr) {
			ip->i_flag &= ~ITEXT;
			break;
		}
	}
	rlstunlock();

}
