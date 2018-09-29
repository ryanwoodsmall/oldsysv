/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rfadmin.c	1.23.3.10"
/*
 *	Kernel daemon for remote-file administration.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/signal.h"
#include "sys/file.h"
#include "sys/fs/s5dir.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/nserve.h"
#include "sys/recover.h"
#include "sys/cirmgr.h"
#include "sys/adv.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/rfsys.h"
#include "sys/tihdr.h"


extern	time_t	time;
extern	int	nservers;		/* current number of servers */
extern	int	idleserver;		/* number of idle servers */
extern	int	msglistcnt;		/* number of msgs in msglist */
extern	int	minserve, maxserve;	/* fewest, most servers in the system */
extern	int	bootstate;		/* state of RFS -- up, down, or inter */
extern struct proc *rec_proc;		/* pointer to recovery process */

int	nzombcnt;			/* how many zombie servers */
rcvd_t	rd_recover;			/* rd for daemon */
struct proc *rfsdp;			/* pointer to daemon process */
struct inode *rfs_cdir;			/* server current directory */
int 	msgflag;			/* create server if set */
struct proc *s_active;			/* pointer to active servers */
struct proc *s_zombie;			/* pointer to zombie children */
sema_t rfadmin_sema;
struct bqctrl_st rfmsgq;		/* queue for user-level daemon */
int	rfmsgcnt;			/* how many msgs in user-level q */
int creatsrv = 0;			/* indicates previous newproc failure */

void reply(), user_msg(), que_umsg();
extern void serv_fumount ();

/*
 *	File sharing daemon, started when file sharing starts.
 */

void
rfdaemon ()
{
	int	i,s,size;
	struct sndd reply_port;
	mblk_t	*bp, *deque();
	queue_t	*qp;
	struct request *request;
	struct response *response;
	mblk_t	*resp_bp;
	char usr_msg [ULINESIZ]; /* tmp space for user daemon message */

	extern int  nservers, msglistcnt;
	extern struct gdp gdp[];
	extern void cl_fumount();
	extern char *strncpy();

	/* disassociate this process from terminal */
	i = 1;
	u.u_ap = &i;
	setpgrp();

	u.u_ap = u.u_arg;

	/* ignore all signals */
	for (i=0; i<NSIG; i++)
		u.u_signal[i] = SIG_IGN;

#ifdef DEBUG
	clrilocks();
#endif
	nzombcnt = 0;
	msgflag = NULL;
	s_zombie = NULL;

	creat_server (minserve);

	/* Make user-daemon queue null. */
	while ((bp = deque (&rfmsgq)) != NULL) {
		DUPRINT1 (DB_RFSYS, "discarding old user-level message\n");
		freemsg (bp);
	}
	rfmsgq.qc_head = (struct msgb *) NULL;
	rfmsgq.qc_tail = (struct msgb *) NULL;
	rfmsgcnt = 0;
	reply_port.sd_stat = SDUSED;
	reply_port.sd_refcnt = 1;
	reply_port.sd_copycnt = 0;
	bootstate = DU_UP;
	wakeup(&bootstate);	/* end of critical section begun in rfstart */

	for (;;) {
		ASSERT(noilocks() == 0);
		/* raise priority because vsema is called from gdpserv */
		s = spl5();
		u.u_error = 0;
		sleep(&rd_recover->rd_qslp, PRIBIO);
	loop:
		ASSERT(noilocks() == 0);
		bp = NULL;
		dequeue (rd_recover, &bp, &reply_port, &size);
		if (nzombcnt)
			clean_proc_table ();
		if(bp) {
			splx(s);
			qp = (queue_t *)((struct message *)bp->b_rptr)->m_queue;
			request = (struct request *) 
				  (bp->b_rptr + sizeof (struct message));
			DUPRINT4(DB_RECOVER, "rfadmin: bp %x qp %x rptr %x\n",
					bp, qp, bp->b_rptr);
			switch (request->rq_opcode ) {
			case REC_FUMOUNT:
				{
				register struct mount *mp;
				mp = &mount[request->rq_mntindx];
				cl_fumount (request->rq_mntindx,
						request->rq_srmntindx);
				freemsg (bp);
				reply (&reply_port, REC_FUMOUNT);
				user_msg (RF_FUMOUNT, mp->m_name, NMSZ);
				break;
				}
			case REC_MSG:
				{
				int size;

				/* Got a message for user-level daemon.
				 * Enque message and wake up daemon.
 				 */
				DUPRINT1 (DB_RFSYS,
				"rfadmin: got a message for user daemon \n");
				size = request->rq_count;
				/* save message so we can free stream buf */
				strncpy (usr_msg, request->rq_data, size);
				freemsg (bp);
				reply (&reply_port, REC_MSG);
				user_msg (RF_GETUMSG, usr_msg, size);
				break;
				}
			case DUSYNCTIME:
				{
				long	sig;
				int	cursig;

				DUPRINT1(DB_RFSYS, "rfadmin: date sync\n");
				GDP(qp)->time = request->rq_synctime - time;
				/* alocbuf and send reply */
				sig = u.u_procp->p_sig;
				cursig = u.u_procp->p_cursig;
				u.u_procp->p_sig = 0;
				u.u_procp->p_cursig = 0;
				while ((resp_bp = alocbuf(sizeof(struct response) - DATASIZE, BPRI_MED)) == NULL) {
					sig |= u.u_procp->p_sig;
					if (u.u_procp->p_cursig) {
						if (cursig)
							sig |= (1L << (u.u_procp->p_cursig - 1));
						else
							cursig = u.u_procp->p_cursig;
					}
					u.u_procp->p_sig = 0;
					u.u_procp->p_cursig = 0;
				}
				u.u_procp->p_sig = sig;
				u.u_procp->p_cursig = cursig;
				response = (struct response *)PTOMSG(resp_bp->b_rptr);
				response->rp_type = RESP_MSG;
				size = sizeof (struct response) - DATASIZE;
		 		response->rp_opcode = DUSYNCTIME;
				response->rp_synctime = time;
				response->rp_errno = 0;
				response->rp_sig = 0;
				freemsg (bp);
				sndmsg (&reply_port, resp_bp, size, 
					(rcvd_t)NULL);
				break;
				}
			case DUCACHEDIS:
				{
				register queue_t *qp;
				long sig;
				int cursig;

				qp = (queue_t *)((struct message *)(bp->b_rptr))->m_queue;
				DUPRINT4(DB_CACHE, "rfadmin: got disable cache message, queue %x, mntindx %x, fhandle %x\n",
				qp, request->rq_mntindx, request->rq_fhandle);
				invalidate_cache(qp, request->rq_mntindx, request->rq_fhandle);
				freemsg (bp);
				/* alocbuf and send reply */
				sig = u.u_procp->p_sig;
				cursig = u.u_procp->p_cursig;
				u.u_procp->p_sig = 0;
				u.u_procp->p_cursig = 0;
				while ((resp_bp = alocbuf(sizeof(struct response) - DATASIZE, BPRI_MED)) == NULL) {
					sig |= u.u_procp->p_sig;
					if (u.u_procp->p_cursig) {
						if (cursig)
							sig |= (1L << (u.u_procp->p_cursig - 1));
						else
							cursig = u.u_procp->p_cursig;
					}
					u.u_procp->p_sig = 0;
					u.u_procp->p_cursig = 0;
				}
				u.u_procp->p_sig = sig;
				u.u_procp->p_cursig = cursig;
				response = (struct response *)PTOMSG(resp_bp->b_rptr);
				response->rp_type = RESP_MSG;
				size = sizeof (struct response) - DATASIZE;
		 		response->rp_opcode = DUCACHEDIS;
				response->rp_errno = 0;
				response->rp_sig = 0;
				(void)sndmsg (&reply_port, resp_bp, size, (rcvd_t)NULL);

				/* link down is the only case that sndmsg
				   can fail, if it happens, server recovery
				   should take care of the losing message */
				break;
				}
			default:
				u.u_error = EINVAL;
				DUPRINT2 (DB_RFSYS, "rfadmin: unknown opcode %d\n",
					request->rq_opcode);
				freemsg (bp);
			}
		} else {
			if (!msgflag) {
				splx(s);
				continue;
			}
			if (msgflag & DISCONN) {
				register struct gdp *tmp;
				msgflag &= ~DISCONN;
				splx(s);
				for(tmp = gdp; tmp < &gdp[maxgdp]; tmp++) 
					if(tmp->flag & GDPDISCONN) {
						cleanup (tmp->queue);
						tmp->flag = GDPRECOVER;
					}
			}
			else if (msgflag & RFSKILL) {
				 DUPRINT1(DB_RECOVER, "rfadmin: KILL case\n");
				msgflag = NULL;
				splx(s);
				signal (rfsdp->p_pgrp,SIGKILL);
				rfsdp = NULL;
				if (rec_proc == NULL) {
					bootstate = DU_DOWN;
					wakeup(&bootstate);
				}
				exit(0);
			}
			else if (msgflag & MORE_SERVE) {
				msgflag &= ~MORE_SERVE;
				splx(s);
				if(idleserver <= 1 && nservers < maxserve) 
					if (creat_server(1) == FAILURE &&
					    idleserver == 0)
						sendback(&reply_port,ENOMEM,RESP_MSG);
			}
			else if (msgflag & DEADLOCK) {
				msgflag &= ~DEADLOCK;
				splx(s);
				if (idleserver == 0 && nservers == maxserve)
					sendback(&reply_port,ENOMEM,NACK_MSG);
			}
		}
		s = spl5();
		goto loop;
	}
}

/*
 *	Create num servers.
 */

creat_server(num)
int num;
{
	int i;

	DUPRINT2 (DB_SERVE,"create %d server(s) \n", num);
	for (i = 0; i < num; i++) {	/* make minimum server */
		switch (newproc(NP_FAILOK|NP_NOLAST|NP_SYSPROC)) {
			case 0:
				nservers++;
				break;
			case 1:
				u.u_cutime = u.u_cstime = u.u_utime = u.u_stime = 0;
				u.u_procp->p_minwd = NULL;
				serve();
			default:
			{
				register int s;
				DUPRINT1(DB_SERVE,"cannot create server \n");
				/* post a signal to the last server so it 
				 * won't go to sleep */
				if (s_active)
					psignal(s_active, SIGUSR1);
				return(FAILURE);
				break;
			}
		} 
	}
	return(SUCCESS);
}

/*
 *	Free zombie servers.
 */

clean_proc_table()
{
	struct proc *curproc;

	DUPRINT2 (DB_SERVE, "clean_proc_table: nzombcnt %d\n", nzombcnt);
	curproc = rfsdp->p_child;
	while (curproc != NULL)  {
		if (curproc->p_stat == SZOMB) {
			freeproc(curproc, 1);
			nzombcnt--;
			curproc = rfsdp->p_child;
		} else 
			curproc = curproc->p_sibling;
	}
}

/*
 *	Send reply with opcode over destination SD.
 */

void
reply (dest, opcode)
sndd_t	dest;		/* reply path */
int	opcode;		/* what we did */
{
	mblk_t	*resp_bp;
	struct response *response;
	int	size;
	long	sig;
	int	cursig;

	/* alocbuf and send reply */
	sig = u.u_procp->p_sig;
	cursig = u.u_procp->p_cursig;
	u.u_procp->p_sig = 0;
	u.u_procp->p_cursig = 0;
	while ((resp_bp = alocbuf(sizeof(struct response) - DATASIZE, BPRI_MED)) == NULL) {
		sig |= u.u_procp->p_sig;
		if (u.u_procp->p_cursig) {
			if (cursig)
				sig |= (1L << (u.u_procp->p_cursig - 1));
			else
				cursig = u.u_procp->p_cursig;
		}
		u.u_procp->p_sig = 0;
		u.u_procp->p_cursig = 0;
	}
	u.u_procp->p_sig = sig;
	u.u_procp->p_cursig = cursig;
	response = (struct response *)PTOMSG(resp_bp->b_rptr);
	response->rp_type = RESP_MSG;
	size = sizeof (struct response) - DATASIZE;
	response->rp_opcode = opcode;
	response->rp_errno = 0;
	response->rp_sig = 0;
	sndmsg (dest, resp_bp, size, (rcvd_t)NULL);
}

/*
 *	Create message for local user-level daemon, and enque it.
 */
 
void
user_msg (opcode, name, size)
long opcode;
char *name;
int  size;
{
	mblk_t	*bp;
	struct u_d_msg *request;

	if ((bp = allocb (sizeof(struct u_d_msg),BPRI_MED)) == NULL) {
		printf ("user_msg allocb fails: ");
		printf ("resource %s has been disconnected \n", name);
		return;
	}
	request = (struct u_d_msg *) bp->b_wptr;
	request->opcode = opcode;
	strcpy (request->res_name, name);
	request->count = size;
	que_umsg (bp);
}

/*
 *	If there's room on user_daemon queue, enque message and wake daemon.
 */

static void
que_umsg (bp)
mblk_t	*bp;
{
	register int s;
	struct u_d_msg *request;

	s = splrf ();
	request = (struct u_d_msg *) bp->b_wptr;
	if (rfmsgcnt > maxgdp) {
		printf ("rfs user-daemon queue overflow: ");
		printf ("make sure rfudaemon is running\n ");
		DUPRINT3 (DB_RFSYS, "opcode %d, name %s\n",
			request->opcode, request->res_name);
		freemsg (bp);
		splx (s);
		return;
	}
	++rfmsgcnt;
	DUPRINT2 (DB_RFSYS,
		"que_umsg: enque msg for user-daemon, count %d\n", rfmsgcnt);
	enque (&rfmsgq, bp);
	splx (s);
	cvsema (&rfadmin_sema);
}

/*
 *This routine is called when flow control is required.
 *It is called with the buffer to be retransmitted along
 *with the send descriptor which is already setup to retransmit
 *to determine the destination.  The buffer type is a 2k type
 *which is a critical resource for the driver. Hence, this buffer
 *is freed and a lesser critical buffer (smaller) can be allocated.
 *One, however, never knows which application may be hogging this 
 *smaller buffer, so if one cannot be allocated, then use the one 
 *you have.
 */

sendback(sd,errno,msgtype)
sndd_t sd;
int errno;
int msgtype;
{
	mblk_t *nbp, *bp;
	mblk_t *sigmsg();
	register struct rcvd *rd;
	register struct response *msg_out;
	register struct message *msg, *nmsg;
	int size, opcode;
	int s;
	/* 
	 * It is wise at this point to try and use a
	 * buffer of different size.  If it is available
	 * then free up the buffer that just ran out of.
	 * If you can't get anther size, reuse the one that 
	 * came in.
	 */
	for (rd = rcvd; (rd < &rcvd[nrcvd]); rd++) {
		if (!(rd->rd_stat & RDUSED) ||
		    !(rd->rd_qtype & GENERAL) || rcvdemp(rd))
			continue;
		if (dequeue (rd, &bp, sd, &size) == SUCCESS) {
			msg = (struct message *)bp->b_rptr;	
			opcode = ((struct request *)PTOMSG(msg))->rq_opcode;
			if ((opcode == DUCLOSE || opcode == DUIPUT) &&
			   !(msg->m_stat & VER1) ) {
				if ((nbp = allocb (sizeof(struct message)+
				     sizeof (struct response)-DATASIZE, BPRI_MED)) != NULL) {
					nbp->b_wptr += sizeof(struct message);
					nmsg = (struct message *)nbp->b_rptr;	
					bcopy ((caddr_t)msg, (caddr_t)nmsg,
					       sizeof(struct message)+
					       sizeof(struct request)-DATASIZE);
					freemsg(bp);
					DUPRINT2(DB_SYSCALL,"Sendback:3.0 DUCLOSE or DUIPUT opcode = %d\n",opcode);
					s=splrf();
					enque(&rd->rd_rcvdq,nbp);
					rd->rd_qcnt++;
					add_to_msgs_list(rd);
					splx(s);
					continue;
				}else {	/* no small buffer enque and continue*/	
					s=splrf();
					enque(&rd->rd_rcvdq,bp);
					rd->rd_qcnt++;
					add_to_msgs_list(rd);
					splx(s);
					continue;
				}
			} 
			if (!(msg->m_stat & GIFT)) {
				if (opcode != DURSIGNAL) {
					freemsg(bp);
					continue;
				}
				if ((nbp = sigmsg(sd,bp)) == NULL)
					continue;
				bp = nbp;   /* message to be signalled */
				msg = (struct message *)bp->b_rptr;	
				opcode = ((struct request *)PTOMSG(msg))->rq_opcode;
				msgtype = RESP_MSG;
				errno = EINTR;
			}
			if ((nbp = allocb (sizeof(struct message)+
			     sizeof (struct response)-DATASIZE, BPRI_MED)) != NULL) {
				freemsg(bp);		 /* free scare buffer */
				bp = nbp;
			}
			bp->b_rptr = bp->b_datap->db_base;
			bp->b_wptr = bp->b_datap->db_base + sizeof(struct message);
			bzero((char *)bp->b_rptr, sizeof(struct message)+
			       sizeof(struct response)-DATASIZE);
			msg_out = (struct response *)PTOMSG(bp->b_rptr);
			msg_out->rp_opcode = opcode;
			msg_out->rp_type = msgtype;
			msg_out->rp_errno = errno;
			msg_out->rp_bp = (long)bp;
			msg_out->rp_sig = 0;
			msg_out->rp_rval = 0;
			msg_out->rp_subyte = 1;	     /* set to indicate no data */
			msg_out->rp_cache = 0;
			size = sizeof (struct response) - DATASIZE;
			DUPRINT2(DB_SYSCALL,"SENDBACK: syscall=%d\n",opcode);

			/*The only time sndmsg fails is if link is down at which point
			 *you really don't care
			*/
			(void)sndmsg (sd, bp, size, (rcvd_t)NULL);
		}
	}
}
/* Note, Even though the signal queue "sigrd" is a high priority queue
 * it must be serviced since large numbers of signal messges could
 * accumulate on the overflow queue and hence cause deadlock.
 * This routine tries to identify the process to be signalled.  If the
 * request is still on the queue, it is taken off and returned.
 * if there is one then send the signal back.
 */
mblk_t *
sigmsg(sd,bp)
struct sndd *sd;
mblk_t *bp;
{
	register struct proc *l;
	mblk_t *sbp;
	mblk_t *chkrdq();
	struct message *msig;
	struct request *msg_in;
	struct rcvd *queue;
	register int s;

	msig = (struct message *)bp->b_rptr;
	msg_in = (struct request *) PTOMSG(msig);
	for (l = s_active; l != NULL; l = l->p_rlink) {
		DUPRINT3(DB_SIGNAL,"DURSIG DEADLOCK: l->p_sysid = %d  u.u_procp->p_sysid %d\n",l->p_sysid,u.u_procp->p_sysid);
		if ((l->p_epid == (short)msg_in->rq_pid)  && 
		    (l->p_sysid == u.u_procp->p_sysid)) { 
			psignal(l, SIGTERM);
			freemsg(bp);
			return(NULL);
		}
	}
	s=splrf();
	queue =  inxtord(msig->m_dest);
	if ((sbp = chkrdq(queue,msg_in->rq_pid,
	     msg_in->rq_sysid)) == NULL) {
		splx(s);	
		freemsg(bp);	/*could not find msg on que */
		return(NULL);
	} else {
		splx(s);
		freemsg(bp);		/*found msg on queue free old buffer*/
		msig = (struct message *)sbp->b_rptr;
		msig->m_stat |= SIGNAL;
		set_sndd(sd,(queue_t *)msig->m_queue,msig->m_gindex,msig->m_gconnid);
		return(sbp);
	}
}
