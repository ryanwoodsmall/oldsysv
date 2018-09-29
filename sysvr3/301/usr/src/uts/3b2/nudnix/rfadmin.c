/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rfadmin.c	1.23"
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
				DUPRINT1(DB_RFSYS, "rfadmin: date sync\n");
				GDP(qp)->time = request->rq_synctime - time;
				/*alocbuf and send reply */
				while ((resp_bp = alocbuf (sizeof (struct response) - DATASIZE, BPRI_MED)) == NULL) {
					u.u_procp->p_sig = 0;
				}
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
					creat_server(1);
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
		switch (newproc(1)) {
			case 0:
				nservers++;
				break;
			case 1:
				u.u_procp->p_flag |= SSYS;
				u.u_cutime = u.u_cstime = u.u_utime = u.u_stime = 0;
				u.u_procp->p_minwd = NULL;
				serve();
			default:
				DUPRINT1(DB_SERVE,"cannot create server \n");
				/* post a signal to the last server so it won't go
				   to sleep */
				if (s_active)
					psignal(s_active, SIGUSR1);
				break;
		} 
	}
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

	/*alocbuf and send reply */
	while ((resp_bp = alocbuf (sizeof (struct response) - DATASIZE, BPRI_MED)) == NULL) {
		u.u_procp->p_sig = 0;
	}
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
