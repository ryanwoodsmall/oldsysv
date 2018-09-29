/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rmount.c	10.39"
/*
 *	remote mount stuff - provide remote access to a
 *	designated part of a file system structure.
 *
 *	NOTE: in six different places in this file, long hand
 *		C syntax was used in place of the short hand notation.
 *		This was done for 7300 compatibility only.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
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
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/adv.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/inline.h"
#include "sys/sysinfo.h"

extern	time_t	time;
extern	int	bootstate;
extern	int	nadvertise;
extern	int	nsrmount;
extern	struct	inode *namei();
extern	struct	advertise *getadv (), *findadv ();
extern	struct	srmnt	*alocsrm ();
extern	rcvd_t	cr_rcvd();
extern 	int	adv_lck;
extern	struct gdp gdp[];
char	*nameptr();
char	*h_alloc();

/*
 *	remote mount - rmount(remote_file_system_name, directory,
 *			name_to_refer_to_other_end_by, rwflag);
 */

rmount()
{
	struct	a {
		char	*rmtfs;		/* name of service (fs) */
		char	*mntpt;		/* directory mount point */
		struct token *token;	/* identifier of remote mach */
		int	rwflag;		/* readonly/read write flag */
	} *uap = (struct a *) u.u_ap;
	register struct	inode *ip = NULL;
	register struct	mount *mp = NULL, *m;	
	char	name[MAXDNAME+1];
	struct token	token;
	char	*to, *from;	
	register struct	request	*request; /*  request message to send out */
	struct	response *resp;		/*  de_queue parameters: response */
	sndd_t	sdp = NULL;
	struct	advertise *ap;
	struct	queue	*qp = NULL;
	queue_t	*get_circuit();
	mblk_t	*bp = NULL, *in_bp;
	extern short dufstyp;

	if (bootstate != DU_UP)  {  /*  have to be on network  */
		u.u_error = ENONET;
		return;
	}

	/* if we are server, uid mapping may make a legitimate request
	 * have a non-root id.  It's secure to skip the check on the 
	 * server because its only role in a remote mount is to handle 
	 * namei for the mount pt., and if namei completes on the server,
	 * the request will fail with EREMOTE anyway.
	 */
	if (!server() && !suser())
		return;
	
	/*  if server, skip directly to the namei, as there is no user space  */
	if (!server ())  {
		/* bring the token into kernel space	*/
		if (copyin(uap->token, &token, sizeof(struct token))) {
			u.u_error = EFAULT;
			DUPRINT1(DB_MNT_ADV,"rmount: copyin failed...\n");
			return;
		}
		DUPRINT3(DB_MNT_ADV,"rmount: token.t_id=%x, t_uname=%s\n",
			token.t_id, token.t_uname);
		/*  bring the advertised name into kernel space  */
		switch (upath(uap->rmtfs,name,MAXDNAME+1)) {
		case -2:	/* too long	*/
		case  0:	/* too short	*/
			u.u_error = EINVAL;
			return;
		case -1:	/* bad user address	*/
			u.u_error = EFAULT;
			return;
		}

		/*  make sure that resource isn't advertised locally  */
		if ((ap = findadv (name)) != NULL) {
			/* Remove this test to allow mount of local resource. */
			if ( !(ap->a_flags & A_MINTER)) {
				u.u_error = EINVAL;
				return;
			}
		}
		if ((qp = get_circuit (-1, &token)) == NULL) {
			DUPRINT3(DB_MNT_ADV,"rmount fails: token.t_id=%x, t_uname=%s\n",
				token.t_id, token.t_uname);
			u.u_error = ENOLINK;
		/*
		 * WARNING - this is the ONLY place rmount() can return this error!
		 */
			return;
		}
		if ((sdp = cr_sndd ()) == NULL) {
			u.u_error = ENOMEM; 
			goto failed;
		}
		set_sndd (sdp, qp, CFRD, 0);
	}
	/*  need to do namei on the second argument  */
	u.u_dirp = uap->mntpt;
	if ((ip = namei(upath, 0)) == NULL) { 
		if (u.u_error == ENOLINK)
			u.u_error = ECOMM;
		goto failed;
	}
	/*  if the namei request succeeded and we're still on the server
	 *  machine, it's an error because can't mount on a remote point.
	 */
	if (server())  {
		u.u_error = EREMOTE;
		goto failed;
	}
	/*  Can only be in the local case here.  Check the type of file.  */
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		goto failed;
	}

  	if (ip->i_count != 1 || (ip->i_flag & IISROOT)) {
		u.u_error = EBUSY;
		goto failed;
	}

	/*  allocate a remote mount table entry, and initialize it.  */
	for (m = NULL, mp = mount; mp < (struct mount *) v.ve_mount; mp++)
		if (mp->m_flags == MFREE) {
			if (!m)
				m = mp;
		}
		else if (mp->m_rflags && !strncmp(name, mp->m_name, MAXDNAME+1)) {
			u.u_error = EBUSY;
			mp = NULL;	/* don't free mount table entry */
			goto failed;
		}
	if (!(mp = m)) {
		u.u_error = EBUSY;
		goto failed;
	}
	mp->m_flags = MINTER;
	mp->m_inodp = ip;
	mp->m_bcount = 0;
	u.u_mntindx = mp - mount;
	if ((mp->m_name = h_alloc(strlen(name)+1)) == NULL) {
		DUPRINT1(DB_MNT_ADV,"rmount: h_alloc failed\n");
		goto failed;
	}

	bp = alocbuf(sizeof (struct request), BPRI_LO);
	if (bp == NULL) {
		u.u_error = EINTR;
		goto failed;
	}
	request = (struct request *) PTOMSG(bp->b_rptr);
	request->rq_type = REQ_MSG;
	request->rq_opcode = DUSRMOUNT;

	tcanon("c0",nameptr(name),request->rq_data,0);
	request->rq_uid = u.u_uid;
	request->rq_gid = u.u_gid;
	request->rq_flag = uap->rwflag;
	request->rq_mntindx = u.u_mntindx;
	request->rq_sysid = GDP(qp)->sysid;
	if (!GDP(qp)->mntcnt)
		request->rq_synctime = time;
	if (rsc (sdp,bp,sizeof(struct request), &in_bp, sdp) != SUCCESS)
		goto failed;
	resp = (struct response *)PTOMSG(in_bp->b_rptr);
	if (resp->rp_errno) {
		u.u_error = resp->rp_errno;
		freemsg(in_bp);
		goto failed;
	} 
	/* this is redundant after the first mount on a circuit */
	if (!GDP(qp)->mntcnt)
	{
	    hibyte(GDP(qp)->sysid) = lobyte(loword(resp->rp_sysid));
	    DUPRINT2(DB_MNT_ADV,"rmount: set sysid to %d\n",GDP(qp)->sysid);
	}
	sdp->sd_mntindx = resp->rp_mntindx;
	mp->m_fstyp = dufstyp;
	mp->m_dev = -1;


	/* get an inode */
	{
	     struct inode *iip, *iget();
	     extern mblk_t *reusebuf();
	     DUPRINT3 (DB_FSS,"rmount:  calling iget mp = %x ino = %d\n",
				mp, sdp->sd_sindex);
	     if((iip = iget (mp, sdp->sd_sindex)) == NULL) {
		bp = reusebuf (in_bp, sizeof (struct request)-DATASIZE);
		request = (struct request *) PTOMSG(bp->b_rptr);
		request->rq_type = REQ_MSG;
		request->rq_opcode = DUSRUMOUNT;
		request->rq_uid = u.u_uid;
		request->rq_gid = u.u_gid;
		request->rq_mntindx = sdp->sd_mntindx;
		freemsg(in_bp);
		if (rsc (sdp, bp, sizeof(struct request)-DATASIZE, &in_bp, (sndd_t)NULL)
		    == SUCCESS) 
			freemsg(in_bp);
		goto failed;
	     }
	     DUPRINT3 (DB_FSS,"rmount: iget ip = %x i_count = %d\n",
				iip,iip->i_count);
	     if (iip->i_fsptr) 
	    	     panic ("rmount inode hit\n");	
	     else {
		     /* populate the independent inode */
		     DUPRINT1 (DB_FSS,"rmount: populate ip \n");
		     iip->i_fsptr = (int *)sdp;
		     iip->i_ftype = IFDIR;
	     }
	     mp->m_mount = iip;
	     prele (iip);
	}

	mp->m_rflags = MDOTDOT;
	mp->m_flags = MINUSE;

	/* success */
	if (uap->rwflag & 1)
		mp->m_flags |= MRDONLY;
	ip->i_mnton = mp;
	strcpy (mp->m_name, name);
	ip->i_flag |= IRMOUNT;
	prele(ip);
	freemsg (in_bp);
	ASSERT (u.u_error == 0);
	DUPRINT3(DB_SYSCALL, "rmount okay: sndd %x, flags=%x\n",
		sdp,mp->m_flags);

	/* bump the mount count */
	GDP(qp)->mntcnt++;

	return;
failed:
	ASSERT (u.u_error != 0);
	if (ip)
		iput (ip);
	if (qp)
		put_circuit (qp);
	if (sdp)
		free_sndd (sdp);
	if (mp) {
		if (mp->m_name) {
			h_free(mp->m_name);
			mp->m_name = NULL;
		}
		mp->m_flags = MFREE;
	}

	DUPRINT2(DB_SYSCALL, "rmount failed: u_error is %d\n", u.u_error);
	return;
}



/*  Server side of the remote mount.
 *  A message (opcode DUSRMOUNT) has come in, and this context
 *  has a u_area filled in with permission information.
 *  The gift to send the answer back on is the other argument.
 *  Send the answer back to the original client.
 */

rcvd_t
srmnt (gp)
struct gdp	*gp;
{
	register struct	a  {
		char	*mdata;
		int	rwflag;
	}  *uap = (struct a *) u.u_ap;
	register struct	inode		*ip;
	register struct	srmnt		*smp;
	register int tmp;
	struct	advertise	*ap;
	char	resname[NMSZ];

	fcanon("c0",uap->mdata,resname);


	if ((ap = findadv(resname)) == NULL) {
		u.u_error = ENODEV;
		return(NULL);
	}
	if (ap->a_flags & A_MINTER) {
		u.u_error = ENONET;
		return(NULL);
	}
	ip = ap->a_queue->rd_inode;
	
	if ((ap->a_flags & A_RDONLY) && !(uap->rwflag & 1)) {
		u.u_error = EROFS;
		return(NULL);
	}
	ap->a_count++;
	plock (ip);
	/* see if client is authorized	*/
	if (ap->a_clist && !checkalist(ap->a_clist,gp->token.t_uname)) {
		u.u_error = EACCES;
		goto out;
	}
	/*  allocate an entry in the srmount table  */
	smp = alocsrm (ip);
	if (u.u_error)  
		goto out;
	tmp = u.u_mntindx;
	u.u_mntindx = smp - srmount;
	if ((cr_rduser (ap->a_queue, u.u_gift->sd_queue)) == NULL) { 
		u.u_error = ENOSPC;
		goto out;
	}
	smp->sr_sysid = u.u_procp->p_sysid;
	smp->sr_rootinode = ip;
	smp->sr_mntindx = tmp;
	smp->sr_flags = MINUSE;
	smp->sr_bcount = 0;
	smp->sr_slpcnt = 0;
	if (uap->rwflag & 1)
		smp->sr_flags |= MRDONLY;
	smp->sr_refcnt = 1;
	ip->i_flag = ip->i_flag | IDOTDOT;
	ip->i_count++;
	prele (ip);
	DUPRINT2(DB_SYSCALL,"srmnt: return rcvd %x\n",ap->a_queue);
	return(ap->a_queue);
out:
	if ((--(ap->a_count) == 0) && ((ip->i_flag & IADV) == 0)) {
		ap->a_flags = A_FREE;
		iput(ip);
	} else 
		prele(ip);
	return(NULL);
}

/*
 *	rumount - unmount a remote file system.
 */
rumount()
{
	register struct	a {
		char	*rmtfs;
	} *uap = (struct a *) u.u_ap;
	char	name[MAXDNAME+1];
	register struct	mount	*mp;
	register struct	request	*request;
	register struct	response *resp;
	register struct sndd	*sd;
	mblk_t	*bp, *in_bp;
	
	if (!suser())
		return;

	if (bootstate != DU_UP)  {  /*  have to be on network  */
		u.u_error = ENONET;
		return;
	}

	switch (upath(uap->rmtfs,name,MAXDNAME+1)) {
	case -2:	/* too long	*/
	case  0:	/* too short	*/
		u.u_error = EINVAL;
		return;
	case -1:	/* bad user address	*/
		u.u_error = EFAULT;
		return;
	}

	for (mp = mount; mp < (struct mount *) v.ve_mount; mp++)
		if ((mp->m_flags & MINUSE) && mp->m_rflags && 
				!strncmp(mp->m_name, name, MAXDNAME+1))
			break;
	if (mp >= (struct mount *) v.ve_mount) {
		u.u_error = EINVAL;
		return;
	}

	mp->m_flags &= ~MINUSE;
	mp->m_flags |= MINTER;
	plock(mp->m_inodp);
	sd = (struct sndd *)mp->m_mount->i_fsptr;
	xumount (mp); /*remove unused sticky files from region tables*/
	if ((icheck (mp) == -1) || (sd->sd_refcnt > 1)) {
		u.u_error = EBUSY;
		goto out1;
	}

	bp = alocbuf(sizeof (struct request)-DATASIZE, BPRI_LO);
	if (bp == NULL) {
		u.u_error = EINTR;
		goto out1;
	}
	request = (struct request *) PTOMSG(bp->b_rptr);
	request->rq_type = REQ_MSG;
	request->rq_opcode = DUSRUMOUNT;
	request->rq_uid = u.u_uid;
	request->rq_gid = u.u_gid;
	request->rq_mntindx = sd->sd_mntindx;
	if (rsc (sd,bp,sizeof(struct request)-DATASIZE,&in_bp, (sndd_t)NULL)
	    != SUCCESS) {
		ASSERT(u.u_error != 0);
		if (u.u_error == ENOLINK) {
			/* link is gone - let the unmount succeed */
			DUPRINT1(DB_MNT_ADV,"rumount succeeds because link is gone\n");
			u.u_error = 0;
			goto success;
		}
		goto out1;
	}
	resp = (struct response *)PTOMSG(in_bp->b_rptr);
	if (resp->rp_errno)  {
		u.u_error = resp->rp_errno;
		freemsg (in_bp);
		goto out1;
	}
	freemsg (in_bp);
	/*  Success - srmnt entry was removed from remote  */
	/*  (or link is down, so let unmount succeed anyway) */
success:
	GDP(sd->sd_queue)->mntcnt--;
	put_circuit (sd->sd_queue);
	mp->m_inodp->i_flag &=  ~IRMOUNT;
	mp->m_inodp->i_mnton = NULL;
	iput(mp->m_inodp);
	/*  get rid of send descriptor pointing to remote inode  */
	plock (mp->m_mount);
	sd->sd_refcnt++;
	iput (mp->m_mount);
	free_sndd (sd);
	mp->m_mount = NULL;
	mp->m_flags = MFREE;
	h_free(mp->m_name);
	mp->m_name = NULL;
	mp->m_rflags = 0;
	return;
out1:	mp->m_flags &= ~MINTER;
	mp->m_flags |=  MINUSE;
	prele(mp->m_inodp);
	return;
}

/*  Server side of remote unmount.
 *  Make sure that the reference count in the srmount table for
 *  this system and this inode is zero.  Then free the srmount entry.
 *  Inode is locked by routines that call this routine.  This routine
 *  is called by server and by recovery.
 */

srumount (smp, ip)
struct	srmnt	*smp;
struct	inode	*ip;
{
	struct	advertise	*ap;

	DUPRINT3 (DB_MNT_ADV, "srumount: entry %x, inode %x \n", smp, ip);
	 /*  still busy for client machine   or  recovery is going on */
	if (smp->sr_refcnt != 1 || smp->sr_flags & MINTER)  { 
		u.u_error = EBUSY;
		return(0);
	}

	/*  Decrement the advertise entry for this resource.
	 *  If it's zero and the advertise bit is off in the inode,
	 *  remove the advertise table entry and remove the queue.
	 */
	ap = getadv (ip);
	ASSERT (ap != NULL);
	if ((--(ap->a_count) == 0) && ((ip->i_flag & IADV) == 0))  {
		ip->i_flag  &= ~IDOTDOT;
		ip->i_count--;
		ap->a_flags = A_FREE;
		ap->a_queue = NULL;
	}
	/* now free up the srmount entry for this machine	*/
	smp->sr_flags = MFREE;
	smp->sr_rootinode = NULL;
	return(1);
}

/*  Srmount table lookup using (inode, sysid) as the key.
 */

struct	srmnt *
getsrmount (ip, sysid)
struct	inode	*ip;
sysid_t	sysid;
{
	register struct	srmnt	*sp;

	for (sp = srmount; sp < &srmount[nsrmount]; sp++)
		if (sp->sr_flags & MINUSE)
		if (sp->sr_sysid == sysid && sp->sr_rootinode == ip) {
			return (sp);
		}
	return (NULL);
}

/*  Find an empty entry in the smount table, make sure that this
 *  machine doesn't already have this directory mounted.
 */

struct	srmnt	*
alocsrm (ip)
struct	inode	*ip;
{
	struct	srmnt	*smp, *sfree;

	for (smp = srmount, sfree = NULL; smp < &srmount [nsrmount]; smp++)  
	{
		if (sfree == NULL && smp->sr_flags == MFREE)  {
			sfree = smp;
			continue;
		}
		if (smp->sr_flags & MINUSE) {
			if (smp->sr_rootinode == ip &&
		    	    smp->sr_sysid == u.u_procp->p_sysid)  {
				u.u_error = EBUSY;
				return(NULL);
			}
		}
	}
	if (!(smp = sfree))  {
		u.u_error = ENOSPC;
		DUPRINT1(DB_MNT_ADV, "srmount table overflow\n");
		return (NULL);
	}
	return (smp);
}

/*  Remote lbin unmount
 */
rlbumount (sd)
sndd_t	sd;
{
	mblk_t	*in_bp;
	mblk_t	*out_bp;
	struct	request	*req;
	sndd_t	new_gift;		/* IGNORED -- from rsc */

	out_bp = alocbuf (sizeof (struct request) - DATASIZE, BPRI_LO);
	if (out_bp == NULL) {
		u.u_error = EINTR;
		return;
	}
	req = (struct request *) PTOMSG(out_bp->b_rptr);
	req->rq_type = REQ_MSG;
	req->rq_opcode = DULBUMOUNT;
	req->rq_mntindx = sd->sd_mntindx;
	dinfo.osyscall++;		/* outgoing (umount) system calls */
	if (rsc (sd, out_bp, sizeof (struct request) - DATASIZE,
		&in_bp, new_gift) != SUCCESS) {
		if (u.u_error == ENOLINK) {
			DUPRINT1 (DB_MNT_ADV,
				"rlbumount succeeds because link is gone\n");
			u.u_error = 0;	/* all OK at user level */
			free_sndd (sd);
		}
		return;
	}
	free_sndd (sd);
	freemsg (in_bp);
	return;
}

/*  Server side of lbin mount
 *
 *  Namei on the other side has detected a remote directory,
 *  so call namei here to complete the pathname evaluation
 *  and set up a new srmount table entry for the mounted-on
 *  directory.
 */

rcvd_t
slbmount ()
{
	struct a  {
		char	*name;
		int	newmntindx;
	}  *uap = (struct a *) u.u_ap;
	register struct	inode	*ip;
	register rcvd_t	gift;
	struct	srmnt	*smp;

	ip = namei (upath, 0);
	if (ip == NULL)
		return(NULL);

	u.u_error = EMULTIHOP;
	iput(ip);
	return(NULL);
}

/***** following code comes back if lbin comes back

	if (ip->i_flag & IRMOUNT)  {
		u.u_error = EBUSY;
		iput (ip);
		return (NULL);
	}
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		iput(ip);
		return(NULL);
	}
	smp = alocsrm (ip);
	if (u.u_error)  {
		iput (ip);
		return (NULL);
	}
	if ((gift = ip->i_rcvd) == NULL)  {
		if ((gift = ip->i_rcvd = cr_rcvd (FILE_QSIZE, GENERAL|RDLBIN)) == NULL) {
			u.u_error = ENOMEM;
			iput(ip);
			return (NULL);
		}
		gift->rd_inode = ip;
		gift->rd_max_serv = FILE_QSIZE;
	}

	smp->sr_sysid = u.u_procp->p_sysid;
	smp->sr_rootinode = ip;
	smp->sr_mntindx = uap->newmntindx;
	smp->sr_dotdot = u.u_mntindx;
	smp->sr_refcnt = 1;
	smp->sr_bcount = 0;
	smp->sr_flags = MINUSE | MLBIN;
	srmount[u.u_mntindx].sr_refcnt++;
	u.u_mntindx = smp - srmount;
	ip->i_flag = ip->i_flag | ILBIN;
	prele(ip);
	return (gift);

}
***** end of old slbmount - comes back if lbin comes back *****/

/*  Server side of lbin unmount.
 *
 *  The other side has checked that there is no activity in the
 *  file system.  This side just has to free the srmount entry.
 *  Inode is locked by routines that call this routine.  This routine
 *  is called by server and by recovery.
 */

slbumount (smp, ip)
struct	srmnt	*smp;
struct	inode	*ip;
{
}

/***** following code comes back if lbin comes back

{
	struct	srmnt	*smatch;

	DUPRINT3 (DB_MNT_ADV, "slbumount: entry %x, inode %x \n", smp, ip);
	ASSERT(smp->sr_refcnt == 1);
	smp->sr_flags = MFREE;
	smp->sr_rootinode = NULL;
	for (smatch = srmount; smatch < &srmount[nsrmount]; smatch++)
 		if ((smatch->sr_flags != MFREE) &&
 		    (smatch->sr_rootinode == ip) &&
 		    (smatch->sr_flags & MLBIN))
			break;
	if (smatch == &srmount [nsrmount])  
		ip->i_flag = ip->i_flag & ~ILBIN;
	return;
}

***** end of old slbumount - comes back if lbin comes back *****/

/*
 *	nameptr returns a pointer to the last element of
 *	a domain name.  E.g., if name == a.b.c, nameptr
 *	returns a ptr to c.  If name == a, it would return
 *	a ptr to a.
 */
char	*
nameptr(name)
register char	*name;
{
	register char	*ptr=name;

	ASSERT(name != NULL);

	while (*name)
		if (*name++ == SEPARATOR)
			ptr = name;

	return(ptr);
}
