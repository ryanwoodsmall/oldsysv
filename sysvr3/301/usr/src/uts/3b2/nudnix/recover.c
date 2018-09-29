/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/recover.c	10.31"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/signal.h"
#include "sys/file.h"
#include "sys/fs/s5dir.h"
#include "sys/param.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/recover.h"
#include "sys/debug.h"
#include "sys/rfsys.h"
#include "sys/rdebug.h"
#include "sys/conf.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"

#define RDUMATCH(P) ((P != NULL) && (P->ru_srmntindx == u.u_mntindx))

rcvd_t	rd_recover;		/* rd for recovery */
struct	rd_user *rdu_frlist;	/* free rd_user structures */
lock_t	rdu_flck;		/* rdu-freelist lock */
int 	recov_off = 0;		/* don't recover if set */
struct	proc *rec_proc;		/* sleep address for recovery */
int	rec_flag;		/* set to KILL when it's time to exit */

extern	int nsrmount;
extern	void rm_msgs_list ();
extern	int	bootstate;
extern	struct	proc *rfsdp;

void cleanup ();
void check_mount ();
void clean_SRDs ();
void check_srmount ();
void wake_serve ();
void clean_sndd ();
void dec_srmcnt ();
void checkq ();
void flushnack ();

/*
 *	Initialize recovery (called at startup time).
 */

void
recover_init ()
{
	register int	i;

	/* Initialize list of free rd_user structures. */
	for (i = 0; i < nrduser - 1; i++) {
		rd_user[i].ru_next = &rd_user[i+1];
	}
	if (nrduser >= 1) {
		rd_user[nrduser - 1].ru_next = (struct rd_user *) NULL;
		rdu_frlist = rd_user;
	} else {
		rdu_frlist = NULL;
	}
	initlock(&rdu_flck,1);
}

/*
 *	Non-sleeping part of recovery:  mark the resources 
 *	that need to be cleaned up, and awaken the recover
 *	daemon to clean them.
 *
 *	This routine is called by the rfdaemon when a circuit
 *	gets disconnected and when a resource is fumounted
 *	(server side of fumount). THIS ROUTINE MUST NOT SLEEP.
 *	It must always be runnable to wake up procs (e.g., servers)
 *	sleeping in resources that have been disconnected.  Otherwise
 *	these procs and the recover daemon can deadlock.
 */

void
cleanup (bad_q)
queue_t	*bad_q;		/* stream that has gone away */
{
	register struct srmnt *sp;
	register int i;
	sysid_t bad_sysid;

	DUPRINT2(DB_RECOVER, "cleanup initiated for queue %x\n",bad_q);

	clean_sndd (bad_q);
	clean_SRDs (bad_q);

	/* Wakeup procs sleeping on stream head - e.g., sndmsg canput failed. */
	wakeup (bad_q->q_ptr);

	bad_sysid = ((struct gdp *) bad_q->q_ptr)->sysid;
	DUPRINT1(DB_RECOVER, "recovery: check srmount table\n");

	/* Mark bad srmount entries. */
	for (sp = srmount, i = 0; sp < &srmount[nsrmount]; sp++, i++) {
		if ((sp->sr_flags & MINUSE) && (sp->sr_sysid == bad_sysid)) {
			DUPRINT2 (DB_RECOVER,
				"\t link down for srmount entry %d \n", i);
			if (!(sp->sr_flags & MINTER)) { /* fumount started */
				wake_serve ((index_t) i);
			}
			sp->sr_flags |= (MINTER | MLINKDOWN);
		}
	}
	rec_flag |= DISCONN;
	wakeup (&rec_proc);
	return;
}

/*
 *	Recovery daemon, awakened by cleanup to clean up after
 *	fumount or disconnect.  This part of recovery calls
 *	routines that can sleep.
 */

recovery ()
{
	int i;
	struct gdp *gdpp;

	rec_flag = 0;
	/* Disassociate this process from terminal. */
	i = 1;
	u.u_ap = &i;
	setpgrp();
	
	/* ignore all signals */
	for ( i = 0; i < NSIG; i++)
		u.u_signal[i] = SIG_IGN;

	for (;;) {
	   while(rec_flag) {
		DUPRINT1 (DB_RECOVER, "recovery daemon awakened \n");
		if (rec_flag & DISCONN) {
			rec_flag &= ~DISCONN;
			for (gdpp = gdp; gdpp < &gdp[maxgdp]; gdpp++)
				if(gdpp->flag & GDPRECOVER)  {
					check_mount(gdpp->queue);
					check_srmount ();
					put_circuit (gdpp->queue);
				  }
		} else if (rec_flag & FUMOUNT) {
			rec_flag &= ~FUMOUNT;
			check_srmount ();
		} else if (rec_flag & RFSKILL) { /* RFS stop */
			DUPRINT1(DB_RECOVER,"recovery daemon exits \n");
			rec_proc = NULL;
			if (rfsdp == NULL) {
				bootstate = DU_DOWN;
				wakeup(&bootstate);
			}
			exit (0);
		}
	   }
	   sleep (&rec_proc, PREMOTE);
	}
}

/*
 *	Go through mount table looking for remote mounts over bad stream.
 *	Send message to user-level daemon for every mount with bad link.
 *	(Kernel recovery works without this routine.)
 */

static void
check_mount (bad_q)
queue_t	*bad_q;
{
	register struct	mount *mp;
	extern void user_msg ();

	DUPRINT1(DB_RECOVER, "recovery: check mount table\n");

	for (mp = mount; mp < (struct mount *) v.ve_mount; mp++) {
		if (!(mp->m_flags & MINUSE))
			continue;

		/* remote mounts */
		if ((mp->m_rflags & MDOTDOT) && 
		    ((((struct sndd *)mp->m_mount->i_fsptr)->sd_queue)==bad_q)){
			DUPRINT2 (DB_RECOVER,
				"\t link down to remote file system %s\n",
				mp->m_name);
			user_msg (RF_DISCONN, mp->m_name, NMSZ);
		}

		/* (Check for lbin mounts if lbin comes back.) */
		
	}
}

/*
 *	Cleanup SPECIFIC RDs:
 *	Wakeup procs waiting for reply over stream that went bad.
 */

void
clean_SRDs (bad_q)
queue_t	*bad_q;		/* stream that has gone away */
{
	register rcvd_t rd;
	index_t srm_index;

	for (rd = rcvd; rd < &rcvd[nrcvd]; rd++)  {
		if (ACTIVE_SRD(rd)&&(bad_q==(struct queue *)rd->rd_user_list)){
			DUPRINT1(DB_RECOVER, "recovery: waking SPECIFIC RD \n");
			srm_index=((struct sndd *)(rd->rd_inode))->sd_mntindx;
			checkq (rd, srm_index);
			rd->rd_stat |= RDLINKDOWN;
			wakeup (&rd->rd_qslp);
		}
	}
}

/*
 *	Check server mount table.  Wake server procs sleeping
 *	in resource that went bad.  Pretend client gave up
 *	references to resource.
 */

static void
check_srmount ()
{
	register struct srmnt *sp;
	register int i;
	register rcvd_t rd;
	queue_t *bad_q, *sysid_to_queue();
	struct inode *ip;

	DUPRINT1(DB_RECOVER, "recovery: check srmount table\n");
	for (sp = srmount, i = 0; sp < &srmount[nsrmount]; sp++, i++) {
		if (sp->sr_flags & (MLINKDOWN | MFUMOUNT)) {
			DUPRINT2 (DB_RECOVER,
			"\t link down for srmount entry %d \n", i);
			/* Wait for servers to wake, leave resource.*/
			while (sp->sr_slpcnt) {
				sleep (&srmount[i], PZERO);
			}
			bad_q = sysid_to_queue (sp->sr_sysid);
			ASSERT(bad_q);
			/* Bump count on inode so it won't go away -
			 * iput in dec_srmcnt when doing unmount */
			ip = sp->sr_rootinode;
			plock (ip);
			ip->i_count++;
			prele (ip);
			/* Now clean up GENERAL RDs */
			for (rd = rcvd; rd < &rcvd[nrcvd]; rd++)  {
				if (ACTIVE_GRD (rd))
					clean_GEN_rd (rd, bad_q, i);
			}
		}
	}

}

/*
 *	On the server side, signal any server process sleeping
 *	in the resource with this srmount index. Count the servers
 *	we signal - we must wait for them to finish.
 */

void
wake_serve (srmindex)
index_t srmindex;
{
	register sndd_t sd;

	for (sd = sndd; sd < &sndd[nsndd]; sd++) 
		if ((sd->sd_stat & SDSERVE) && (sd->sd_stat & SDUSED) &&
		    (sd->sd_mntindx == srmindex) && (sd->sd_srvproc != NULL)) {
			DUPRINT1(DB_RECOVER, "wake_serve: waking server \n");
			sd->sd_stat |= SDLINKDOWN;
			psignal (sd->sd_srvproc, SIGTERM);
			++srmount[srmindex].sr_slpcnt;
		}
}

/*
 *	Link is down.  Mark send-descriptors that use it.
 */

static void
clean_sndd (bad_q)
queue_t	*bad_q;
{
	register sndd_t sd;

	DUPRINT1(DB_RECOVER, "recovery: marking send-descriptors\n");
	for (sd = sndd; sd < &sndd[nsndd]; sd++) 
		if ((sd->sd_stat & SDUSED) && (sd->sd_queue == bad_q)) {
			DUPRINT2(DB_RECOVER, "   sd %x link is down\n",sd);
			sd->sd_stat |= SDLINKDOWN;
		}
}

/*
 *	Clean up GENERAL RD.
 *
 *	Traverse rd_user list of this RD.  For each rd_user from
 *	this srmount index, pretend that client gave up all refs
 *	to this RD.
 *
 *	(Need bad_q to get sysid for cleanlocks.)
 */

void
clean_GEN_rd (rd, bad_q, srm_index)
rcvd_t	rd;
queue_t	*bad_q;
index_t srm_index;
{
	register int i;
	struct	inode *ip;
	struct rd_user *rduptr, *rduptr1;
	queue_t *rduser_queue;
	struct srmnt *smp;
	sysid_t oldsysid;

	DUPRINT4 (DB_RECOVER, "clean_GEN RD: rd %x, refcnt %d, inode %x\n",
				rd, rd->rd_refcnt, rd->rd_inode);
	rduptr = rd->rd_user_list;
	ASSERT (rduptr != NULL);

	checkq (rd, srm_index); /* get rid of old messages */
	/* clean up SDs on nacker queue */
	if(rd->rd_sdnack)
		flushnack (rd, srm_index);
	ip = rd->rd_inode;

	/* cleanlocks needs sysid - save old sysid */
	oldsysid = u.u_procp->p_sysid;
	u.u_procp->p_sysid = ((struct gdp *)bad_q->q_ptr)->sysid;

	/* Traverse rd_user list. */

	while (rduptr != NULL) {
		rduptr1 = rduptr->ru_next;

		if ((rduptr->ru_srmntindx != srm_index)) {
			/* don't clean this user - get next one */
			rduptr = rduptr1;
			continue;
		} 

		/* Clean this reference. */
 
		rduser_queue = rduptr->ru_queue;
		/* index into srmount table - for del_rduser */
		u.u_mntindx = srm_index;
		smp = &srmount[srm_index];

		/* Mimic what a server would do to get rid of reference. */

		/* Clean up FILE uses of this resource. */
		if (rduptr->ru_fcount > 0) {
			u.u_syscall = DUCLOSE;
			plock(ip);
			cleanlocks(ip,IGN_PID);
			prele(ip);
			DUPRINT1 (DB_RECOVER, "  FS_CLOSEI \n");
			if((rduptr->ru_frcnt == 0) && (rduptr->ru_fwcnt == 0)) {
				plock (ip);
				FS_CLOSEI (ip, FREAD|FWRITE, 1, 0);
				prele (ip);
			} else {
				for (i = rduptr->ru_frcnt; i > 0; --i) {
					plock(ip);
					FS_CLOSEI (ip, FREAD, 1, 0);
					prele(ip);
					rduptr->ru_frcnt--;
				}
				for (i = rduptr->ru_fwcnt; i > 0; --i) {
					plock(ip);
					FS_CLOSEI (ip, FWRITE, 1, 0);
					prele(ip);
					rduptr->ru_fwcnt--;
				}
			}
			rduptr->ru_fcount = 0;
		}

		/* Clean up INODE uses of this resource. */
		for (i = rduptr->ru_icount; i > 0; --i) {
			DUPRINT1 (DB_RECOVER, "  clean inode \n");
			u.u_syscall = DUIPUT;
			dec_srmcnt (smp, bad_q);
			plock (ip);
			del_rcvd (rd, rduser_queue);
			iput (ip);
		}

		rduptr = rduptr1;
	}	 /* end while */

	/* restore old sysid - else this proc will look like server */
	u.u_procp->p_sysid = oldsysid;
}

/*
 *	Throw away SDs with this srmindx from the nacker queue of this RD.
 */

void
flushnack (rd, srmindx)
register rcvd_t rd;
register int srmindx;
{
	register int s;
	register sndd_t tmpsd;
	register sndd_t sd;

	s = splrf();
	sd = rd->rd_sdnack;
	tmpsd = sd->sd_next;
	while (tmpsd  != NULL) {
		if (tmpsd->sd_mntindx == srmindx) {
			sd->sd_next = tmpsd->sd_next;
			free_sndd(tmpsd);
			tmpsd = sd->sd_next;
		} else 
			sd = tmpsd;
			tmpsd = tmpsd->sd_next;
	}
	/* now check the first one */
	sd = rd->rd_sdnack;
	tmpsd = sd->sd_next;
	if (sd->sd_mntindx == srmindx) {
		rd->rd_sdnack = tmpsd;
		free_sndd(sd);
	}
	splx (s);
}

/*
 *	Decrement the reference count in the srmount table.
 *	If it goes to zero, do the unmount.
 *
 *	(This routine does not handle LBIN case.)
 */

static void
dec_srmcnt (smp, bad_q)
struct srmnt *smp;
queue_t *bad_q;
{
	struct inode *ip;
	struct gdp *gdpp;

	if (smp->sr_refcnt > 1) { /* srumount wants refcnt of 1 */
		--smp->sr_refcnt;
		return;
	}

	/*
	 * Giving up last ref for this srmount entry - free it.
	 * The inode we're working on in clean_GEN is NOT necessarily
	 * the root of the resource we're unmounting.
	 */

	ip = smp->sr_rootinode;
	DUPRINT2 (DB_RECOVER, "recovery freed srmount entry %x \n",smp);
	plock (ip);
	smp->sr_flags &= ~MINTER;
	srumount (smp, ip);
	/* One extra iput for bump in check_srmount and fumount. */
	iput (ip);

	/* Client usually cleans up circuit, but client is gone. */
	gdpp = GDP (bad_q);
	--gdpp->mntcnt;
}

/*
 *	Giving away this rd as a gift.  Return an rd_user structure
 *	to keep track of it.  If we've already given this rd to the
 *	same client (for the same srmount index), use the same structure.  
 *	Otherwise point rd at new structure and return it.
 */

static struct rd_user *
alloc_rduser (rd)
register struct rcvd *rd;
{
	register struct rd_user *rduptr;

	rduptr = rd->rd_user_list;
	for ( ; rduptr != NULL; rduptr = rduptr->ru_next)
		if (RDUMATCH (rduptr)) /* already exists - return old one */
			return (rduptr); 

	/* need a new one */
	if ((rduptr = rdu_frlist) == NULL) {
		printf ("alloc_rduser: out of rd_user space\n");
		return (NULL);
	}
 	/* u_mntindx tells which srmount entry request is for */
	rduptr->ru_srmntindx = u.u_mntindx;
	rduptr->ru_icount = 0;
	rduptr->ru_fcount = 0;
	rduptr->ru_frcnt = 0;
	rduptr->ru_fwcnt = 0;
	rdu_frlist = rdu_frlist->ru_next;

	/* insert at head of list */
	rduptr->ru_next = rd->rd_user_list;
	rd->rd_user_list = rduptr;
	return (rduptr);
}

/*
 *	Free an rd_user structure.
 */

static void
free_rduser (rduptr)
struct rd_user *rduptr;
{
	DUPRINT1 (DB_RDUSER, "  free_rduser \n");
	rduptr->ru_next = rdu_frlist;
	rdu_frlist = rduptr;
	return;
}

/*
 *	Giving away an RD.  Keep track of who's getting it and why.
 *	This routine must be called every time a remote machine
 *	gets a reference to a resource.
 */

struct rd_user *
cr_rduser (rd, qp)
rcvd_t	rd;
queue_t	*qp;		/* stream head - i.e., who's getting gift */
{
	register struct rd_user *rduptr;
	struct rd_user *alloc_rduser();
	register struct inode *ip;
	ip = rd->rd_inode;

	if ((rduptr = alloc_rduser(rd)) == NULL)
		return (NULL);

	DUPRINT4 (DB_RDUSER, " cr_rduser: rd %x, qp %x, srmntindx %d \n",
			rd, qp, u.u_mntindx);
	rduptr->ru_queue = qp;
	if (u.u_syscall == DUOPEN) {
		rduptr->ru_fcount++;
		/* for fifo case, bump reader/writer counts */
		if (ip->i_ftype & IFIFO) {
			if (u.u_arg[1] & FREAD)
				rduptr->ru_frcnt++;
			if (u.u_arg[1] & FWRITE)
				rduptr->ru_fwcnt++;
		}
	} else if (u.u_syscall == DUCREAT) {
		rduptr->ru_fcount++;
		if (ip->i_ftype & IFIFO) 
			rduptr->ru_fwcnt++;
	}
	rduptr->ru_icount++;
	return (rduptr);

}

/*
 *	A client is giving up a reference to this RD; decrement
 *	count in rd_user struct we made in cr_rduser.  If count
 *	goes to zero, free the struct.
 */

void
del_rduser (rd)
rcvd_t	rd;
{
	register struct rd_user *rduptr;
	register struct rd_user *rdut;
	void free_rduser();

	DUPRINT4 (DB_RDUSER,
		" del_rduser: rd %x, srmntindx %d, mode %x\n",
			rd, u.u_mntindx, u.u_fmode);


	rduptr = NULL;
	ASSERT (rd->rd_user_list != NULL);  /* no users to delete */

	if (RDUMATCH(rd->rd_user_list)) {	/* matched first */
		rduptr = rd->rd_user_list;
		goto done;
	}
	for (rdut = rd->rd_user_list;	/* not first - traverse list */
		 (!RDUMATCH (rdut->ru_next) && (rdut != NULL));
			rdut = rdut->ru_next);

	ASSERT (rdut != NULL); /* try to delete user that's not there */
	rduptr = rdut->ru_next;

done:
	if (u.u_syscall == DUCLOSE) {
		rduptr->ru_fcount--;	
		if (rd->rd_inode->i_ftype & IFIFO) {
			if (u.u_fmode & FREAD)
				rduptr->ru_frcnt--;
			if (u.u_fmode & FWRITE)
				rduptr->ru_fwcnt--;
		}
		return;
	}
	if (--rduptr->ru_icount) 
		return;

	/* last reference - get rid of rd_user struct */
	if (rd->rd_user_list == rduptr)
		rd->rd_user_list = rduptr->ru_next;
	else
		rdut->ru_next = rduptr->ru_next;
	free_rduser(rduptr);
	return;
}

/*
 *  An srmount entry went bad (disconnect or fumount),
 *  and recovery is cleaning it up.  Throw away any old
 *  messages that are on this rd for the bad entry.
 */

void
checkq (rd, srmindex)
rcvd_t	rd;
index_t srmindex;
{
	register int i, qcnt, s;
        mblk_t  *bp;
        mblk_t  *deque();
	index_t msg_srmindex;
	struct message *message;
	struct request *request;

	DUPRINT1 (DB_RECOVER, "recovery: checkq \n");
        s = splrf(); /* arrmsg() calls enque() at stream priority */

	qcnt = rd->rd_qcnt;
	for (i = 0; i < qcnt; i++) {
        	bp = deque (&rd->rd_rcvdq);
		ASSERT (bp != NULL);
		message = (struct message *) bp->b_rptr;
		request = (struct request *) PTOMSG (message);
		msg_srmindex = request->rq_mntindx;
		if (msg_srmindex == srmindex) {
			DUPRINT2 (DB_RECOVER, "   toss bp %x\n", bp);
			/* don't service this message - toss it */
			freemsg (bp);
			rd->rd_qcnt--;
                        if (rcvdemp(rd)) {
                                rm_msgs_list (rd);
				break;
			}
		} else {
			/* message OK - put it back */
	        	enque (&rd->rd_rcvdq, bp);
		}
        }
	splx(s);
	return;
}


