/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/fumount.c	10.24"
/*
 *	Forced unmount - get rid of all remote users of a resource.
 *	Forced unmount is unusual because SERVER originates request.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/adv.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/cmn_err.h"
#include "sys/recover.h"

void fu_servers ();
void fu_SRDs ();

extern rcvd_t rd_recover;
extern int msgflag;
extern struct proc *rec_proc;

extern void wake_serve ();
extern void checkq();

/*
 *	Forced unmount - called from rfsys system call.
 *	Go through srmount table, looking for clients
 *	that have the resource mounted.  Send each such
 *	client the fumount message.  (Send it to admin RD.)
 *	Then call recovery to pretend the client gave up all
 *	references to the resource.
 */

void
fumount ()
{
	struct uap {
		int opcode;
		char *resource;		/* resource being fumounted */
	} *uap;

	register int i;
	char	name[DIRSIZ];
	struct	srmnt *smp;
	struct	advertise *ap, *findadv();
	struct	inode *ip;
	queue_t *cl_queue, *sysid_to_queue();
	sndd_t	sdp;
	rcvd_t  rd;

	extern	int	nsrmount;
	extern  int 	rec_flag;
	extern	int	bootstate;
	mblk_t	*bp;

	if (!suser())
		return;

	if (bootstate != DU_UP)  {
		u.u_error = ENONET;
		return;
	}

	uap = (struct uap *) u.u_ap;
	if (copyin(uap->resource, name, DIRSIZ+1)) {
		DUPRINT1 (DB_MNT_ADV,"fumount copyin failed\n");
		u.u_error = EFAULT;
		return;
	}

	if ((ap = findadv (name)) == NULL) {
		DUPRINT1 (DB_MNT_ADV, "fumount findadv failed\n");
		u.u_error = ENOENT;
		return;
	}

	if(!(ap->a_flags & A_MINTER)) {
		DUPRINT1 (DB_MNT_ADV,"fumount: resource is advertised\n");
		u.u_error = EADV;
		return;
	}
	ip = ap->a_queue->rd_inode;
	if ((sdp = cr_sndd ()) == NULL) {
		u.u_error = ENOMEM;
		DUPRINT1 (DB_MNT_ADV, "fumount: cannot create sd\n");
		return;
	}
	if ((rd = cr_rcvd (FILE_QSIZE, SPECIFIC)) == NULL) {
		DUPRINT1 (DB_MNT_ADV, "fumount: cannot create rd\n");
		u.u_error = ENOMEM;
		free_sndd (sdp);
		return;
	}
	if((bp = alocbuf(sizeof(struct request)-DATASIZE, BPRI_LO)) == NULL){
		u.u_error = EINTR;
		free_sndd (sdp);
		free_rcvd (rd);
		return;
	}

	DUPRINT2 (DB_MNT_ADV,"fumount: inode %x\n", ip);

	for (smp = srmount, i = 0; smp < &srmount[nsrmount]; smp++, i++) {
		if ((smp->sr_flags & MINUSE) && (smp->sr_rootinode == ip)) {
			/* this client uses resource - send message */
			if(smp->sr_flags & MINTER) {
				/* already taken care of */
				continue;
			} 
			smp->sr_flags |= MINTER;
			/* prevent new uses - checked by server */
			cl_queue = sysid_to_queue (smp->sr_sysid);
			sdp->sd_stat = SDUSED;
			set_sndd (sdp, cl_queue, RECOVER_RD, RECOVER_RD);
			wake_serve ((index_t) i);
			if (fumount_msg(sdp,smp->sr_mntindx,i,bp,rd)==FAILURE)
				continue;
			/* client got message - clean up */ 
			fu_servers (i);	/* wake up servers in rmove */
			/* wakeup procs sleeping on stream head -
			 * e.g., sndmsg canput failed */
			wakeup (cl_queue->q_ptr);
			smp->sr_flags |= MFUMOUNT;
			rec_flag |= FUMOUNT;
			wakeup (&rec_proc);
		}
	}

	/* If all fumount_msgs succeeded, we are now guaranteed that
	 * no client will send any new request for this resource.
	 */
	freemsg (bp);	
	free_sndd (sdp);
	free_rcvd (rd);
}

/*
 *	Construct and send fumount msg and wait for reply.
 *	Return SUCCESS or FAILURE.
 *
 *	We got sd, rd, and bp in advance, so the ONLY reason
 *	this routine can fail is that the link is down, in
 *	which case recovery cleans up.
 */

fumount_msg (sdp, cl_mntindx, serv_srmntindx, bp, rd)
sndd_t sdp;
index_t cl_mntindx;	/* index into client mount table */
index_t	serv_srmntindx; /* index into server mount table on server */
mblk_t	*bp;
rcvd_t rd;
{
	struct	request	*request;
	mblk_t	 *in_bp;
	int 	size;
	extern	mblk_t *reusebuf();
 
	DUPRINT3 (DB_MNT_ADV,
		"fumount_msg: cl_indx %d, serv_indx %d \n",
		cl_mntindx, serv_srmntindx);

	bp = reusebuf (bp, sizeof ( struct request) - DATASIZE);
	/* keep track of sd we go out on in case link goes down */
	rd->rd_inode = (struct inode *) sdp;
	request = (struct request *) PTOMSG(bp->b_rptr);
	request->rq_type = REQ_MSG;
	request->rq_opcode = REC_FUMOUNT;
	request->rq_mntindx = cl_mntindx;
	request->rq_srmntindx = serv_srmntindx;
	if (sndmsg(sdp, bp, sizeof(struct request) - DATASIZE, rd) == FAILURE) 
		return (FAILURE);
	if (de_queue (rd, &in_bp, (struct sndd *)NULL, &size ) == FAILURE)
		return (FAILURE);
	freemsg (in_bp);
	return (SUCCESS);
}

/*
 *	Find the stream (queue) to client with sysid.
 */

queue_t *
sysid_to_queue (sysid)
sysid_t sysid;
{
	register struct gdp *gdpp;
	extern struct gdp gdp[];

	DUPRINT2 (DB_MNT_ADV, "convert sysid %x to queue \n", sysid);
	for (gdpp = gdp; gdpp < &gdp[maxgdp] ; gdpp++) {
		if (gdpp->sysid == sysid) {
			DUPRINT2 (DB_MNT_ADV, "  queue is %x \n", gdpp->queue);
			return (gdpp->queue);
		}
	}

	cmn_err (CE_PANIC,"sysid_to_queue: no match for sysid 0x%x", sysid);
}


/*
 *	Mark the RDs of server processes sleeping in fumounted resource,
 *	and wake up the servers.
 */

static void
fu_servers (srm_index)
index_t srm_index;
{
	register rcvd_t rd;

	for (rd = rcvd; rd < &rcvd[nrcvd]; rd++)  {
		if ((ACTIVE_SRD (rd)) && (rd->rd_inode) &&
	    	    (((struct sndd *)(rd->rd_inode))->sd_stat & SDSERVE) &&
	    	    (((struct sndd *)(rd->rd_inode))->sd_mntindx==srm_index)){
			DUPRINT1(DB_MNT_ADV, "fumount: waking server \n");
			rd->rd_stat |= RDLINKDOWN;
			checkq (rd, srm_index);
			wakeup (&rd->rd_qslp);
		}
	}
}

/*
 *	Client side of forced unmount.  Mark SDs that point into
 *	resource being force-unmounted.  Wake up processes waiting
 *	for reply over this mount point.
 */

void
cl_fumount(cl_index, serv_index)
index_t	cl_index;	/* index into client mount table */
index_t serv_index;	/* index into server mount table on server */
{
	register sndd_t sd;
	register rcvd_t rd;
	queue_t *fumount_q;

	/* Bad stream head is in SD in mount table. */
	fumount_q=((struct sndd *)(mount[cl_index].m_mount->i_fsptr))->sd_queue;
	DUPRINT4 (DB_MNT_ADV,
		"cl_fumount: queue %x, cl_indx %d, serv_indx %d\n",
		fumount_q, cl_index, serv_index);

	for (sd = sndd; sd < &sndd[nsndd]; sd++) 
		if ((sd->sd_stat & SDUSED) &&
		    (sd->sd_queue == fumount_q) &&
		    (sd->sd_srvproc == NULL) &&
		    (sd->sd_mntindx == serv_index)) {
			DUPRINT2 (DB_MNT_ADV, "   sd %x fumounted\n",sd);
			sd->sd_stat |= SDLINKDOWN;
		}

	/* RD waiting for reply has SD in rd_inode field - see rsc and rmove. */
	for (rd = rcvd; rd < &rcvd[nrcvd]; rd++)  {
		sd = (struct sndd *)(rd->rd_inode);
		if (ACTIVE_SRD (rd) && (sd) && (sd->sd_queue == fumount_q) &&
		   (sd->sd_mntindx == serv_index)) {
			DUPRINT2 (DB_MNT_ADV, 
			"fumount: waking SRD for srmindx %d\n", serv_index);
			rd->rd_stat |= RDLINKDOWN;
			vsema(&rd->rd_qslp,0);	 
		}
	}
}

