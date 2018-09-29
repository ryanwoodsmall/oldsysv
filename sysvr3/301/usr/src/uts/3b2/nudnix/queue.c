/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/queue.c	10.21"
/*
 *	q u e u e . c
 *
 *  A msg is the frozen instance of a system call
 *  that was begun on the local machine and, because it
 *  referenced a remote file, needs to be completed
 *  on a remote machine.
 *
 *  Msgs are received by queues.  queues either belong to the
 *  general pool of queues, which is handled as a unit,
 *  or they are handled individually.
 *
 *  A process can either wait for a msg on a specific queue
 *  or for any msg from the general pool of queues.
 */
#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
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
#include "sys/mount.h"
#include "sys/region.h"
#include "sys/recover.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/rdebug.h"

#define p_minwdlock			p_trlock

void		add_to_proc_list ();
void		del_from_proc_list ();
void		rm_proc_list ();
struct	proc	*get_free_proc();
void		rm_msgs_list ();

/*  Here are the heads of the procs and msgs list.
 *  procs points to the list of processes waiting for work (wait_for)
 *  msgs points to the list of receive queues with pending msgs.
 */
struct proc * procs = NULL;	/* processes waiting for work   */
struct rcvd * msgs = NULL;	/* messages waiting for servers */
extern int msglistcnt;		/* number of rcvd's on message list */
extern int idleserver;		/* number of idle servers */

/*
 * de_queue:  this is the interface to wait_for and dequeue.
 *
 *	wait for a msg to arrive.
 *	dequeue the msg.
 *	if (the queue became empty)
 *		then take the queue off the msgs list
 *		(assuming it was there).
 *	if (the queue is not empty)
 *	 	then try to dispatch another proc
 *
 */
rcvd_t
de_queue (rqueue, bufp, new_gift, size)
rcvd_t	rqueue;
mblk_t	**bufp;
sndd_t	new_gift;
int	*size;
{

	rcvd_t queue;
	int sig;
	int	s;
	extern rcvd_t sigrd;

	if (rqueue != (rcvd_t) ANY)  {
		/* This is were we have to worry about remote signals.
		 * If we were awakened and return with a 1, we've got
		 * a signal and a signal message should be sent.  
		 * Otherwise the wakeup was (presumably) because of an incoming
		 * message.  The extra test is historical
		 */

		sig = u.u_procp->p_sig;
		if (u.u_procp->p_flag & SRSIG)
			u.u_procp->p_sig = 0;

		while (1) {
			s = splrf();
			if (rqueue->rd_qcnt) {
				splx(s);
				break;
			}
			if (rqueue->rd_stat & RDLINKDOWN) {
				DUPRINT2(DB_GDPERR, "de_queue: try to sleep on dead rd %x\n", rqueue);
				u.u_error = ENOLINK;
				splx(s);
				return(FAILURE);
			}
			if (psema(&rqueue->rd_qslp, PREMOTE|PCATCH) != 0) {
				splx(s);
				if (!server()) {
					if (!sysent[u.u_syscall].sy_setjmp) {
						if (!(u.u_procp->p_flag & SRSIG)) {
							DUPRINT3(DB_SIGNAL,"de_queue: op=%s,p_sig=%d\n",
							sysname(u.u_syscall), sig);
							if (sendrsig() == FAILURE)
								return(FAILURE);
						}
					}
				}
				else 
					if (rqueue->rd_stat & RDLINKDOWN)
						return (FAILURE);
			} else {
				splx(s);
				DUPRINT2(DB_COMM, "de_queue: waken up on rd %x", rqueue);
				if (rcvdemp(rqueue)) {
					if (rqueue->rd_stat & RDLINKDOWN) {
						u.u_error = ENOLINK;
						return(FAILURE);
					} else 
						DUPRINT1(DB_SERVE, "de_queue: found empty queue\n");
				}
				else
					break;
			}
			sig |= u.u_procp->p_sig;
			u.u_procp->p_sig = 0;
		}	
		u.u_procp->p_sig = sig;
		dequeue(rqueue,bufp,new_gift,size);
		return(rqueue);
	}
	else  {
back:
		queue = NULL;
		s = splrf();
		if (u.u_procp->p_minwd != NULL)  {
			/* we have finished with our previous msg
			 * so, one less proc is now serving that queue.
			 * skip this when we first start running.
			 */
			u.u_procp->p_minwd->rd_act_cnt--;
			u.u_procp->p_minwd = NULL;
		}
		/* need to raise priority because arrmsg() calls add_to_msgs_list()
		   at stream priority */
		if (sigrd->rd_rcvdq.qc_head != NULL) {
			u.u_procp->p_minwd = sigrd;
			queue = sigrd;
			goto out;
		}
		if (msgs != NULL)  {
			queue = msgs;
			u.u_procp->p_minwd = queue;
			goto out;
		}
		/* Need to raise priority because arrmsg() calls get_free_proc()
		   at stream priority */
		if (idleserver > minserve) {
			splx(s);
			return(NULL);		/* kill itself */
		}
		add_to_proc_list(u.u_procp);
		/* This is the only place servers can be killed.
		 * The epid is reset so that we don't accidentally
		 * try to signal this server
		 */	  
		if (psema(&u.u_procp->p_minwdlock, PREMOTE|PCATCH)) {
			/* return to serve for exit */
			del_from_proc_list (u.u_procp);
			splx(s);
			return (NULL);
		}
		/* we may get here by non-catchable signals or by wakeup in arrmsg */
		del_from_proc_list (u.u_procp);
		splx(s);
		goto back;
	}
out:
	u.u_procp->p_minwd->rd_act_cnt++;
	splx (s);
	if (dequeue(queue,bufp,new_gift,size) != SUCCESS) {
		DUPRINT2(DB_SERVE, "de_queue: queue %x empty\n", queue);
		goto back;
	}
	return (queue);
}

/*
 * Add procs to the BEGINNING of the process list.
 * the procs list is a list of the processes waiting
 * for work from the pool of general queues.  They get
 * on the list by executing wait_for(ANY).
 */
void
add_to_proc_list (procid)
struct proc *procid;
{
	procid->p_rlink = procs;
	procs = procid;
	return;
}


/*
 * A server process is about to exit - remove it from
 * list of available servers.
 */
void
del_from_proc_list (badproc)
struct proc *badproc;
{
	struct proc *procp, *procp2;

	DUPRINT2(DB_SERVE, "del_from_proc: proc %x\n", badproc);
	if (procs == NULL) 
		return;

	if (procs->p_pid == badproc->p_pid) {	/* the one we want is first */
		DUPRINT1(DB_SERVE, "del_from_proc_list: proc deleted\n");
		procs = procs->p_rlink;
		return;
	}

	procp = procs;
	do {				/* not first - traverse list */
		procp2 = procp;
		procp = procp->p_rlink;
	} while ((procp != NULL) && (procp->p_pid != badproc->p_pid));

	if (procp == NULL) {
		DUPRINT1(DB_SERVE, "del_from_proc_list: proc not found\n");
		return;
	}

	DUPRINT1(DB_SERVE, "del_from_proc_list: proc deleted\n");
	procp2->p_rlink = procp->p_rlink;
	return;
}


/*
 * remove the process from the available servers list.
 */

struct proc *
get_free_proc ()
{
	struct proc *found;
	extern int msgflag;
	extern rcvd_t rd_recover;

	found = procs;
	if (found != NULL)
		procs = found->p_rlink;
	return (found);	
}


/*
 * add receive queues to the END of the msgs list.
 * duplicates are ignored because interrupts on
 * different receive descriptors can try to put
 * the same receive queue on the msgs list.
 */
void
add_to_msgs_list (rcvdp)
 rcvd_t rcvdp;
{
	rcvd_t current;

	if (msgs == NULL)  {
		msgs = rcvdp;
		rcvdp->rd_next = NULL;
		msglistcnt = 1;
	}
	else	{
		current = msgs;
		while ((current != rcvdp)
			&& (current->rd_next != NULL))
			current = current->rd_next;
		if (current != rcvdp) { 
			current->rd_next = rcvdp;
			rcvdp->rd_next = NULL;
			msglistcnt++;
		}
	}
	return;
}

/*
 * remove the queue from the msgs list.
 */
void
rm_msgs_list (rcvdp)
 rcvd_t rcvdp;
{
	rcvd_t prev, current;

	if (!(rcvdemp(rcvdp)))
		return;
	if (msgs != NULL) {
		prev = current = msgs;
		if (msgs == rcvdp) {
			msgs = msgs->rd_next;
			msglistcnt--;
		} else  {
			while (((current = prev->rd_next) != NULL) 
				&& (current != rcvdp))
					prev = current;
			if (current != NULL) {
				prev->rd_next = current->rd_next;
				msglistcnt--;
			}
		}
	}
	return;
}

