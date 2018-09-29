/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/remcall.c	10.27"
/*
 *	R E M O T E   C A L L
 *
 *	Format a msg representing the i/o system call that
 *	hit a remote mount point.  Send the msg to the
 *	remote system for further execution.
 *
 *	If successful, remote_call will do a long jump
 *	and so will never return to its caller.
 *
 *	If, however, the pathname evaluation ends up back on
 *	the local system, then remote_call resets u.u_nextcp to the
 *	remainder of the pathname and returns a pointer
 *	to the inode in which the search should continue
 *	locally.
 *
 *	If there is any error, u.u_error is set and NULL returned.
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
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/comm.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/stat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/sysinfo.h"
#include "sys/idtab.h"
#include "sys/stream.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/rdebug.h"
#include "sys/nami.h"
#include "sys/fstyp.h"

extern	time_t	time;
extern	struct	inode *rem_inode(); 

struct inode *
remote_call (ip, arg)
register struct inode *ip;
caddr_t arg;
{
	register struct sndd *out_port = (struct sndd *)ip->i_fsptr;
	mblk_t	*in_bp;
	register mblk_t	*out_bp;
	register struct	request	*msg_out;
	struct	response  *msg_in;
	sndd_t	ret_gift = NULL;/* gift w/in-bound msg		*/
	struct	inode *inode_p;	/* return value		 */
	int	c;		/* c and cp are used to move the */
	register char	*cp;	/* pathname to the msg_out buffer*/
	int	jump = 0;	/* whether to longjmp at end  */

	DUPRINT3(DB_SYSCALL, "remote_call: syscall is %d (%s)\n",
		u.u_syscall, sysname(u.u_syscall));
	if (server()) {	/* if server, this is multihop	*/
		u.u_error = EMULTIHOP;
		return(NULL);
	}
	sysinfo.remcall++;
	out_port->sd_refcnt++;	/* rmount will not succeed if cnt > 1 */
	out_bp = alocbuf(sizeof (struct request), BPRI_LO);
	if (out_bp == NULL) {
		u.u_error = EINTR;
		goto out;
	}
	msg_out = (struct request *) PTOMSG(out_bp->b_rptr);
	msg_out->rq_type = REQ_MSG;
	msg_out->rq_mntindx = out_port->sd_mntindx;
	cp = msg_out->rq_data;
	while (c = *u.u_nextcp) {
		if (cp < &msg_out->rq_data[DATASIZE - 2])
			*cp++ = c;
		else {
			u.u_error = E2BIG;
			freemsg (out_bp);
			goto out;
		}
		u.u_nextcp++;
	}
	*cp++ = '\0';	/*  namei on the other side looks one char past the
			    end of the string.  Don't take this out!!*/
			
	rcallmsg (msg_out, out_port, &ret_gift,arg);
	if (u.u_error) {
		freemsg (out_bp);
		goto out;
	}
	dinfo.osyscall++;	/* outgoing system calls */

	/* send the message and wait for the response */
	rsc (out_port, out_bp, cp - (char *) msg_out,
		&in_bp, ret_gift);
	if (u.u_error) {
		if (ret_gift)
			free_sndd(ret_gift);
		goto out;
	}
	out_port->sd_refcnt--;
	jump = rcallret (in_bp, ret_gift, &inode_p, out_port, ip);
	/* Ensure u.u_nextcp doesn't point to a slash. */
	while (*u.u_nextcp == '/')
		u.u_nextcp++;
	if (jump) {
		u.u_rflags |= U_RSYS;
		longjmp (u.u_qsav, 1);
	} else 
		return (inode_p);
out:
	out_port->sd_refcnt--;
	return (NULL);
}

/*  Prepare a remote call message to send to the server side.
 *  The arguments depend on the call that is continuing remotely.
 *  For operations that are just plain namei's, make sure that
 *  the right intention (sought, create, delete) is made clear.
 */

static
rcallmsg (rmp, out_port, gift, arg)
register struct	request	*rmp;
sndd_t out_port, *gift;
caddr_t arg;
{
	extern short dufstyp;

	rmp->rq_sysid = get_sysid(out_port); 
	rmp->rq_uid = u.u_uid;
	rmp->rq_gid = u.u_gid;
	rmp->rq_opcode = u.u_syscall;
	rmp->rq_ulimit = u.u_limit;
	rmp->rq_cmask = u.u_cmask;

	if (u.u_rdir && (u.u_rdir->i_fstyp == dufstyp))  
		/* put in index of remote root rcvd */
		rmp->rq_rrdir = 
			((sndd_t)(u.u_rdir->i_fsptr))->sd_sindex;
	else 
		rmp->rq_rrdir = 0;

	/*  system calls that do namei's to do more later  */
	switch (u.u_syscall)  {
	case DUEXEC:
	case DUEXECE:
		dinfo.osysexec++;	/* outgoing exec's */
	case DUCHDIR:
	case DUCHROOT:
		if ((*gift = cr_sndd()) == NULL)
			u.u_error = ENOMEM;
		break;
			
	case DULINK1:
	case DULINK:
		if (u.u_pdir == NULL) {	
			if ((*gift = cr_sndd()) == NULL)
				u.u_error = ENOMEM;
		} 
		else {
			register struct inode *tip = u.u_pdir;
			register struct sndd *tsd;

			u.u_syscall = DULINK1;
			rmp->rq_opcode = u.u_syscall;
			if (tip->i_fstyp == dufstyp) {
				tsd = (struct sndd *)tip->i_fsptr; 
				if (tsd->sd_queue == out_port->sd_queue) 
					rmp->rq_link = tsd->sd_sindex;
				else	
					rmp->rq_link = 0;
			} else
				rmp->rq_link = 0;
		}
		break;
	case DUUNLINK:
	case DUIUPDATE:
	case DUUPDATE:
		break;

	case DUCLOSE:
	{
		register struct a {
			int mode;
			int count;
			off_t off;
		} *temp;

		temp = (struct a *)arg;
		rmp->rq_fmode = temp->mode;
		rmp->rq_foffset = temp->off;
		rmp->rq_count  = temp->count;
		break;
	}
	case DUACCESS:
	{
		struct a { char *fname; int fmode; } *uap =
		  (struct a *) u.u_ap;
		rmp->rq_fmode = uap->fmode;
		break;
	}
	case DUADVERTISE:
		u.u_error = EREMOTE;	/* can't adv remote resource */
		break;
	case DUCHMOD:
	{
		struct a { char *fname; int fmode; } *uap =
		  (struct a *) u.u_ap;
		rmp->rq_fmode = uap->fmode;
		break;
	}
	case DUCHOWN:
	{
		register struct a { char *fname; int uid; int gid; } 
		*uap = (struct a *) u.u_ap;
		rmp->rq_newuid = uap->uid;
		rmp->rq_newgid = uap->gid;
		break;
	}
	case DUCOREDUMP:
		if ((*gift = cr_sndd()) == NULL)
			u.u_error = ENOMEM;
		rmp->rq_cmask = u.u_cmask;
		break;
	case DUOPEN:
	{
		register struct a { char *fname; int mode; int crtmode; } 
		*uap = (struct a *) u.u_ap;
		if ((*gift = cr_sndd()) == NULL) {
			u.u_error = ENOMEM;
			return;
		}
		rmp->rq_mode = uap->mode;
		rmp->rq_crtmode = uap->crtmode;
		rmp->rq_cmask = u.u_cmask;
		break;
	}
	case DUCREAT:
	{
		register struct a { char *fname; int fmode; } 
		*uap = (struct a *) u.u_ap;
		if ((*gift = cr_sndd()) == NULL) {
			u.u_error = ENOMEM;
			return;
		}
		rmp->rq_fmode = uap->fmode;
		rmp->rq_cmask = u.u_cmask;
		break;
	}
	case DULBMOUNT:
		if ((*gift = cr_sndd()) == NULL) {
			u.u_error = ENOMEM;
			return;
		}
		rmp->rq_newmntindx = u.u_mntindx;  /*  set in smount()  */
		break;
	case DUMKDIR:
	{
		register struct a { char *fname; int fmode;  }
		*uap = (struct a *) u.u_ap;
		rmp->rq_fmode = uap->fmode;
		break;
	}
	case DUMKNOD:
	{
		register struct a { char *fname; int fmode; dev_t pad; 
		dev_t dev; } *uap = (struct a *) u.u_ap;
		rmp->rq_fmode = uap->fmode;
		rmp->rq_dev = uap->dev;
		rmp->rq_cmask = u.u_cmask;
		break;
	}
	case DURMDIR:
		break;
	case DUMOUNT:
	case DUUMOUNT:
		u.u_error = EREMOTE;
		break;
	case DURMOUNT:
	{
		struct a { char *mntfs; char *mntpt; } *uap=
		  (struct a *) u.u_ap;
		u.u_dirp = uap->mntpt;
		break;
	}
	case DUSTAT:
	{
		struct a { char *fname; struct stat *bs; } *uap =
		  (struct a *) u.u_ap;
		rmp->rq_bufptr = (int) uap->bs;
		break;
	}
	case DUSTATFS:
	{
		register struct a {
			char	*fname;
			struct	statfs *bufp;
			int	len;
			int	fstyp;
		} *uap = (struct a *) u.u_ap;

		rmp->rq_bufptr = (int) uap->bufp;
		rmp->rq_len = uap->len;
		rmp->rq_fstyp = uap->fstyp;
		break;
	}
	case DUUTIME:
	{
		register struct a { char *fname; time_t *tptr; }  
		*uap = (struct a *) u.u_ap;
		if ((*gift = cr_sndd()) == NULL)
			u.u_error = ENOMEM;
		rmp->rq_bufptr = (int) uap->tptr;
		rmp->rq_prewrite = 0;
		break;
	}

	case DUSYNCTIME:
		rmp->rq_synctime = time;
		break;

	default:
		printf ("rcallmsg: syscall type %d unexpected\n", u.u_syscall);
		u.u_error = EINVAL;
	}
	return;
}


/*  return from remote call.
 *  Put arguments back into this process's u area.
 *  Return argument tells remcall whether to do longjump.
 */

#define JUMP 1
#define NOJUMP 0

static
rcallret (in_bp, gift, inop, out_port, ip)
mblk_t	*in_bp;
sndd_t	gift;
struct inode **inop;	
sndd_t	out_port;
register struct inode *ip;
{
	int jump;
	char *data;
	struct	response *resp;

	resp = (struct response *) PTOMSG(in_bp->b_rptr);
	u.u_procp->p_sig |= resp->rp_sig;

	DUPRINT2(DB_SIGNAL,"rcallret:rp_sig=%x\n",resp->rp_sig);
	if (resp->rp_errno != 0) {
		DUPRINT2(DB_SYSCALL, "rcallret: remote u.u_error %d\n",
			resp->rp_errno);
		u.u_error = resp->rp_errno;
		u.u_rval1 = resp->rp_rval;

		if (gift) {
			free_sndd (gift);
		}
		*inop = NULL;
		freemsg (in_bp);
		return (NOJUMP);
	}

	/* Check for crossing machine boundary (ECROSSMNT). */
	if ((resp->rp_opcode == DUDOTDOT) || (resp->rp_opcode == DULBIN)) {
		u.u_nextcp -= strlen(resp->rp_data);
		u.u_rflags |= U_DOTDOT;
		if (resp->rp_opcode == DUDOTDOT)
			*inop =  mount[resp->rp_mntindx].m_inodp;
		else {
			*inop =  mount[resp->rp_mntindx].m_mount;
		}
		plock (*inop);
		(*inop)->i_count++;
		if (gift) {
			free_sndd (gift);
		}
		freemsg (in_bp);
		return (NOJUMP);
	}
	jump = JUMP;
	*inop = NULL;
	data = resp->rp_data;

	if (gift)  
		gift->sd_mntindx = resp->rp_mntindx;
		
	switch (u.u_syscall)  {
	case DUEXEC:
	case DUEXECE:
	case DUCHDIR:
	case DUCHROOT:
	case DULBMOUNT:
	case DUOPEN:
	case DUCREAT:
	case DUCOREDUMP:
	case DULINK:
		*inop = rem_inode (ip, gift, in_bp);
		jump = 0;
		break;
	case DUCLOSE:
	case DUIUPDATE:
	case DULINK1:
	case DUUPDATE:
		jump = 0;
	case DUCHMOD:
	case DUCHOWN:
	case DUMKNOD:
	case DUUNLINK:
	case DURMDIR:
	case DUMKDIR:
		freemsg (in_bp);
		break;

	case DUUTIME:
		free_sndd (gift);
		freemsg (in_bp);
		break;

	case DUACCESS:
	{
		register int	svuid, svgid;
		/*  switch uid, gid with ruid, rgid  */
		svuid = u.u_ruid;
		svgid = u.u_rgid;
		u.u_ruid = u.u_uid;
		u.u_rgid = u.u_gid;
		u.u_uid = svuid;
		u.u_gid = svgid;
		freemsg (in_bp);
		break;
	}
	case DUSTAT:
	{
		register struct stat	*statptr;
		register struct gdp	*gdpp;
		extern struct gdp gdp[];

		statptr = (struct stat *) resp->rp_data;
		gdpp = GDP(out_port->sd_queue);
		hibyte(statptr->st_dev) = ~(gdpp - gdp);
		if (copyout (resp->rp_data, resp->rp_bufptr,
				sizeof (struct stat)) < 0)
			u.u_error = EFAULT;
		freemsg (in_bp);
		break;
	}
	case DUSTATFS:
		if (copyout(resp->rp_data, resp->rp_bufptr, resp->rp_count))
			u.u_error = EFAULT;
		freemsg (in_bp);
		break;

	case DUSYNCTIME:
		GDP(out_port->sd_queue)->time = resp->rp_synctime - time;
		jump = 0;
		freemsg (in_bp);
		break;

	default:
		printf ("rcallret: unknown system returned %d\n",
			u.u_syscall);
		freemsg (in_bp);
				
	}  /*  end switch  */

	return (jump);
}

/*  This should be called only by a server process, in iget() and namei().
 *  A mount point was crossed, return to the server level of the remote
 *  call and send back the mount index and the reason for the crossing.
 *  In the DOT_DOT case, need to back up u.u_nextcp to point to the '..'.
 */

goback (mntindx, why)
int	mntindx;	/*  index into the srmount index	*/
int	why;		/*  LBIN or DOT_DOT			*/
{
	register char c;

	if (why == DOT_DOT) {	/*  point u.u_nextcp to the first '.'  */
		/*
		 * Back up over "..".  Note that a terminating slash may have
		 * been zapped to NUL by namei().
		 */
		while ((c = *--u.u_nextcp) == '/' || c == '\0')
			*u.u_nextcp = '/';
		u.u_nextcp--;
		u.u_error |= EDOTDOT;
	} else		  /*  LBIN */
		u.u_error |= ELBIN;
	u.u_mntindx = mntindx;
}


/* Send an interrupt message across the network.  Finds the
 * correct place to send the message from the minwd field (this means
 * this won't work with servers).  Does not wait for a reply because
 * the reply is a natural (albeit) interrupted return from the remote
 * system call.
 */

sendrsig()
{
	register struct message *smsg;
	register struct request *req;
	register mblk_t	*bp;
	register struct sndd *sd;

	while ((bp = alocbuf(sizeof(struct request)-DATASIZE, BPRI_MED)) == NULL) {
		u.u_procp->p_sig = 0;
	}
	req = (struct request *) PTOMSG(bp->b_rptr);
	sd = (struct sndd *)u.u_procp->p_minwd;
	req->rq_type = REQ_MSG;
	req->rq_opcode = DURSIGNAL;	/* this is a interrupt message */
	req->rq_sysid = get_sysid(sd); 
	req->rq_mntindx = sd->sd_mntindx;
	smsg = (struct message *)bp->b_rptr;
	smsg->m_stat |= SIGNAL;
	if (sndmsg(sd, bp, sizeof(struct request) - DATASIZE,
	 (rcvd_t)NULL) == FAILURE)
		return (FAILURE);
	u.u_procp->p_flag |= SRSIG;  /*so rsc() knows proc is signalled*/	
	return (SUCCESS);
}


/* server() comes here from iget() when ip->i_flag & ILBIN is true */
/* return code: 0: server crossing mount point, go back 
		1: continue on this server machine */
riget(ip)
register struct inode *ip;
{
	register struct	srmnt	*smp;
	extern struct srmnt *getsrmount(); 

 	if (smp = getsrmount (ip, u.u_procp->p_sysid))  {
		goback (smp->sr_mntindx, LBIN);
		strcpy (u.u_arg[0], u.u_nextcp);
		return (0);
	}
	return (1);
}

struct inode *
rem_inode (ip, gift, in_bp)
register struct inode *ip;
register struct sndd *gift;
register mblk_t *in_bp;
{
	register struct inode *iip; 
	register struct response *resp;
	struct inode *iget();
	struct inode tip;
	ushort mode;

	resp = (struct response *) PTOMSG(in_bp->b_rptr);
	tip.i_uid = gluid(GDP(gift->sd_queue), resp->rp_uid);
	tip.i_gid = glgid(GDP(gift->sd_queue), resp->rp_gid);
	tip.i_nlink = resp->rp_nlink;
	tip.i_size = resp->rp_size;
	tip.i_ftype = resp->rp_ftype;
	mode = resp->rp_mode;
	freemsg (in_bp);
	if ((iip = iget (ip->i_mntdev, gift->sd_sindex)) == NULL) {
		del_sndd (gift);
		return (NULL);
	}
	DUPRINT3 (DB_FSS,"rem_inode: iget ip = %x i_count = %d\n",iip,iip->i_count);
	iip->i_uid = tip.i_uid;
	iip->i_gid = tip.i_gid;
	iip->i_nlink = tip.i_nlink;
	iip->i_size = tip.i_size;
	iip->i_ftype = tip.i_ftype;
	if (iip->i_fsptr) {
		DUPRINT2 (DB_FSS,"rcallret: cache hit sndd = %x\n",
			iip->i_fsptr);
                if (gift->sd_sindex!=((struct sndd *)iip->i_fsptr)->sd_sindex) {
                        printf ("rem_inode: panic ip = %x fsptr = %x fstyp = %d gift %x\n",iip,iip->i_fsptr,iip->i_fstyp,gift);
                        panic ("rem_inode cache hit error");
                }
		del_sndd (gift);
		if (u.u_error) {
			iput (iip);
			return (NULL);
		}
	} else {
		/* populate the independent inode */
		DUPRINT1 (DB_FSS,"rcallret: populate ip \n");
		iip->i_fsptr = (int *)gift;
	}
	if ((u.u_syscall == DUEXEC) || (u.u_syscall == DUEXECE))
		((struct sndd *)(iip->i_fsptr))->sd_mode = mode;
	return (iip);
}


duustat()
{
	register struct inode ip;
	register struct a { char *cbuf; int dev; int cmd; } 
	*uap = (struct a *) u.u_ap;
	register struct mount *mp;
	extern struct gdp gdp[];
	short dev;
	sndd_t sdp;
	short index;
	queue_t *qp;
	if ((sdp = cr_sndd ()) == NULL) {
		u.u_error = ENOMEM;
		return(-1);
	}
	ip.i_fsptr = (int *) sdp;
	dev = (short) uap->dev;
	index = (short)((~(hibyte (dev))) & 0x00ff) ;
	if(index >= maxgdp || !(gdp[index].flag & GDPCONNECT)) {
		u.u_error = ENOENT;
		free_sndd(sdp);
		return(-1);
	}
	mp = (struct mount *) lobyte (dev);
	qp = gdp[index].queue;
	DUPRINT4 (DB_FSS,"duustat: mp %x qp %x sdp %x\n", mp, qp, sdp);
	set_sndd (sdp, qp, CFRD, 0);
	remfileop (&ip,NULL,NULL);
	free_sndd(sdp);
}



rem_date ()
{
	register struct gdp *tmp;

	DUPRINT1(DB_SYSCALL, "rem_date: starting to update links\n");
	for (tmp = gdp; tmp < &gdp[maxgdp]; tmp++)
		if ((tmp->queue != NULL) && (tmp->flag & GDPCONNECT)) 
			date_sync (tmp->queue);
}

date_sync (qp)
queue_t *qp;
{
	register sndd_t sd;
	register short tmp;
	register struct inode ip;

	if ((sd = cr_sndd ()) == NULL) { 
		u.u_error = ENOMEM;
		return;
	}
	set_sndd (sd, qp, RECOVER_RD, RECOVER_RD);
	tmp = u.u_syscall;
	u.u_syscall = DUSYNCTIME;
	ip.i_fsptr = (int *)sd;
	remote_call (&ip, NULL);
	u.u_syscall = tmp;
	free_sndd (sd);
}
