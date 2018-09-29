/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/fileop.c	10.17.5.15"

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fcntl.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/stat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/sysinfo.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/buf.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5macros.h"
#include "sys/rbuf.h"

#define MAXRETRY 10

/*  remote file operation
 *  Set up arguments from u area (later using common message builder)
 *  and send a message across requesting the operation.  If it's a
 *  write, send the first DATASIZE bytes of data.
 *  The message that comes back could have the answer to the request,
 *  or may need to transfer some information into or from user space.
 *  So COPYIN and COPYOUT messages may come back from the server as
 *  part of this system call.  The operation is done when the "more"
 *  flag is clear in the response.
 */

remfileop (ip,flag, offset)
register struct inode *ip;
register flag;
register offset;
{
	register struct sndd *sd = (struct sndd *)ip->i_fsptr;
	mblk_t	*bp;
	mblk_t	*in_bp;
	register struct	request	*req;	/* request message to the server */
	register struct	response *resp;	/* response message from the server */
	int retval;
	register int	size = 0;	/*for de_queue, received size		*/
	long loffset = u.u_offset;	/* used by client caching */
	long count = u.u_count;		/* used by client caching */
	long base = (long)u.u_base;		/* used by client caching */
	int retry = 0;			/*retry limit */
	int unlocked = 0;		/*need to plock indicator*/
		
	sndd_t	gift = NULL;		/* return value of de_queue */
	register int	ret;		/* return instead of longjmp */

	DUPRINT3(DB_SYSCALL, "remfileop: syscall = %d (%s)\n", 
			u.u_syscall, sysname(u.u_syscall));
	if (server()) {	/* if on server, this is multihop	*/
		u.u_error = EMULTIHOP;
		return(FAILURE);
	}
	sysinfo.fileop++;
resend:
	u.u_offset = loffset;
	u.u_count = count;
	u.u_base = (caddr_t)base;
	/* get a buffer for the request
	 *
	 * WARNING:
	 *   Before the message is sent out, 'rftocanon' routine
	 *   is called to canonize the data. The canonical data usually
	 *   needs more space. This implementation assumes the STREAMS
	 *   mechanism will always allocate a 2K buffer when a
	 *   sizeof(struct request) buffer is asked for. If your STREAMS
	 *   implementation behaves differently, you may want to
	 *   change the next "alocbuf" statement to ask for more space.
	 *   e.g.  bp = alocbuf(sizeof(struct request) + n, BPRI_LO);
	 *   where 'n' depends on the request type.
	 *   In particular, allocate more space for the GETDENTS request.
	 *
	 *   This problem will be fixed in a later RFS revision.
	 *  -- end of WARNING
	 */
	bp = alocbuf(sizeof (struct request), BPRI_LO);
	if (bp == NULL) {
		u.u_error = EINTR;
		retval = FAILURE;
		goto out;
	}
	req = (struct request *)PTOMSG(bp->b_rptr);
	req->rq_type = REQ_MSG;
	/*  fill in u arguments  */
	req->rq_opcode = u.u_syscall;
	req->rq_sysid = get_sysid(sd);
	req->rq_uid = u.u_uid;
	req->rq_gid = u.u_gid;
	req->rq_ulimit = u.u_limit;
	req->rq_mntindx = sd->sd_mntindx;
	if (sd->sd_stat & SDMNDLCK ) 
		req->rq_flags |= RQ_MNDLCK;
	else
		req->rq_flags &= ~RQ_MNDLCK;
	if ((size = filluargs (bp, &gift, flag, offset, ip)) == -1) {
		freemsg (bp);
		retval = FAILURE;
		goto out;
	}
	dinfo.osyscall++;		/* outgoing system calls */

	/*  send the message and wait for the response  */
	if (rsc (sd, bp, sizeof (struct request) - DATASIZE + size,
             &in_bp, gift) == FAILURE)  {
		if (gift) free_sndd (gift);
		retval = FAILURE;
		goto out;
	}

	/*  now take the arguments out of the message and put back  into
	 *  u area.
	 */
	resp = (struct response *)PTOMSG(in_bp->b_rptr);
	if (resp->rp_errno == ENOMEM && resp->rp_type == NACK_MSG) {
		if (resp->rp_cache & RP_MNDLCK && unlocked == 0) {
			sd->sd_stat |= SDMNDLCK;
			prele(ip);
			unlocked = 1;
		}
		if (retry++ < MAXRETRY) {
			if (gift)
				free_sndd(gift);
			freemsg(in_bp);
			DUPRINT4(DB_SYSCALL,"pid = %d opcode=%d, RETRY = %d \n",u.u_procp->p_pid,u.u_syscall,retry);
			goto resend;
		}
	}
	ret = replyuargs (resp,in_bp,ip);
	/*
	 * If the server returned a gift (probably as a result of some
	 * ioctl or fcntl), allocate a local file descriptor for it.
	 */
	if (((struct message *)in_bp->b_rptr)->m_stat & GIFT) {
		struct inode *nip, *rem_inode();

		gift->sd_mntindx = resp->rp_mntindx;
		if ((nip = rem_inode(ip, gift, in_bp)) != NULL)
			copen1(nip, FREAD);
	} else {
		if (gift)
			free_sndd(gift);
		freemsg(in_bp);
	}
	if (u.u_error)
		ret = TRUE;

	/* Normally longjmp back to trap.c/syscall - we's done.
	 * readi/writei and errors return.
	 */
	if (ret) {
		retval = SUCCESS;
		goto out;
	}
	else  {
		u.u_rflags |= U_RSYS;
		if ( unlocked) {
			plock(ip);
			if (rcacheinit)
				rfinval(sd, -1, 0);
		}
		longjmp (u.u_qsav, 1);
	}

out:
	if ( unlocked) {
		plock(ip);
		if (rcacheinit)
			rfinval(sd, -1, 0);
	}
	return (retval);
	
	
}

/*  set values in the message based on the system call.
 *  These are the file descriptor system calls.
 *  The server recreates the file descriptor before calling the system call.
 */

static
filluargs (bp, gift, flag, offset, ip)
mblk_t	*bp;
sndd_t	*gift;
register flag, offset;
register struct inode *ip;
{
	register struct sndd *sd = (struct sndd *)ip->i_fsptr;
	register struct	request	*req;
	register int	size = 0; /* how much more than sizeof struct request in msg */

	req = (struct request *)PTOMSG(bp->b_rptr);
	switch (u.u_syscall)
	{
	case	DUFCNTL:
	{
		register struct a { int fdes; int cmd; int arg; } 
		*uap = (struct a *) u.u_ap;
		if ((*gift = cr_sndd ()) == NULL){
			u.u_error = ENOMEM;
			return(-1);
		}
		/*
		 * For performance, send record locking data with request.
		 * F_FREESP uses the same structure, so let it go along
		 * for the ride.
		 */
		switch (uap->cmd) {
		case	F_GETLK:
		case	F_SETLK:
		case	F_SETLKW:
		case	F_CHKFL:
		case	F_FREESP:
			size = sizeof (struct flock);
			size = (size > DATASIZE) ? DATASIZE:size;
			req->rq_prewrite = size;
			if (copyin((caddr_t)uap->arg, req->rq_data, size)) {
				u.u_error = EFAULT;
				return(-1);
			}
			flag |= FRCACH;
			break;
		} /*end cmd switch*/

		req->rq_cmd = uap->cmd;
		req->rq_fcntl = uap->arg;
		req->rq_fflag = flag;
		req->rq_foffset = offset;
		break;
	}
	case	DUFSTAT:
	{
		register struct a {
			int fdes; 
			struct stat *bs;
		} *uap = (struct a *) u.u_ap;

		req->rq_bufptr = (int) uap->bs;
		break;
	}
	case	DUSEEK:
		req->rq_whence = 2;
		break;
	case	DUFSTATFS:
	{
		register struct a {
			char	*fd;
			struct	statfs *bufp;
			int	len;
			int	fstyp;
		} *uap = (struct a *) u.u_ap;

		req->rq_bufptr = (int) uap->bufp;
		req->rq_len = uap->len;
		req->rq_fstyp = uap->fstyp;
		break;
	}
	case	DUGETDENTS:
	{
		register struct	a { int fdes; char *cbuf; unsigned count; } 
		*uap = (struct a *) u.u_ap;
		if ((*gift = cr_sndd()) == NULL) {
			u.u_error = ENOMEM;
			return (-1);
		}
		req->rq_offset = u.u_offset;
		req->rq_base = (int) uap->cbuf;
		req->rq_count = uap->count;
		break;
	}
	case	DUIOCTL:
	{
		register struct a { int fdes; int cmd; caddr_t arg; } 
		*uap = (struct a *) u.u_ap;

		if((*gift = cr_sndd()) == NULL){
			u.u_error = ENOMEM;
			return(-1);
		}
		req->rq_cmd = uap->cmd;
		req->rq_ioarg = (int) uap->arg;
		req->rq_fflag = flag;
		break;
	}
	case    DUUTSSYS:  /*  only ustat() gets this far  */
	{
		register struct a { char *cbuf; int dev; int cmd; } 
		*uap = (struct a *) u.u_ap;
		short dev;
		dev = (short)uap->dev;
		req->rq_dev =  lobyte(dev);
		req->rq_bufptr = (int) uap->cbuf;
		break;
	}
	case 	DUREAD:
	case	DUWRITE:
	case	DUREADI:
	case	DUWRITEI:
		if ((*gift = cr_sndd()) == NULL) {
			u.u_error = ENOMEM;
			return (-1);
		}
		req->rq_base = (int) u.u_base;
		req->rq_offset = u.u_offset;
		req->rq_count = u.u_count;
		req->rq_fmode = u.u_fmode;
		/*  if writei, copy the first DATASIZE bytes into msg	*/
		if (u.u_syscall == DUWRITEI || u.u_syscall == DUWRITE)  {
			size = (u.u_count > DATASIZE) ? DATASIZE : u.u_count;
			req->rq_prewrite = size;  /* amount of data present */
			if (u.u_segflg != 1)  {
				if (copyin (u.u_base, req->rq_data,
						size) < 0)  {
					u.u_error = EFAULT;
					free_sndd (*gift);
					return(-1);
				}
			} else	
				bcopy (u.u_base, req->rq_data, size);

			/* check if write data can be put in cache */
			if (u.u_rcstat & U_RCACHE) {
				int blkno, blkct, csize;
				char *from;
				struct rbuf *rbp;
				rcinfo.clwrite += ct_to_blkct(u.u_count,u.u_offset);
				sd->sd_offset = u.u_offset;
				blkno = off_to_blkoff(sd->sd_offset);
				blkct = ct_to_blkct(size, sd->sd_offset);
				csize = size;
				from = (char *)req->rq_data;
				if ((rbp = rget_cache(sd,blkno,blkct)) != NULL)
					rcache_in(rbp, sd, from, csize);
				else {
					rcinfo.cbwrite += blkct;
					if ((sd->sd_offset & RBMASK) == 0) {
						/* whole block pre-write data */
						while (csize >= RBSIZE) {
							if ((rbp=rgeteblk(sd,blkno++)) 
							   != NULL)
								rcache_in(rbp, sd, from, RBSIZE);
							else
								sd->sd_offset +=
								   RBSIZE;
							from += RBSIZE;
							csize -= RBSIZE;
						}
					}
					if (csize > 0)
						sd->sd_offset += csize;
				}
			}
		}
		if (u.u_syscall == DUREAD)
			dinfo.osysread++;	/* outgoing read's */
		else if (u.u_syscall == DUWRITE)
			dinfo.osyswrite++;	/* outgoing write's */
		else
			dinfo.osyscall--;	/* not a sect. 2 syscall; rescind inc */
		break;	
	}  /*  end switch  */
	return (size);
}

/*  Depending on the system call, set necessary u area arguments to 
 *  finish the system call.
 * 
 *  Return value FALSE tells remfileop to do longjump.
 *  Return value TRUE causes return to caller.
 */

static
replyuargs (resp,in_bp,ip)
struct	response *resp;
mblk_t	*in_bp;
register struct inode *ip;
{
	register struct sndd *sd = (struct sndd *)ip->i_fsptr;
	register int ret = FALSE;  /* TRUE for readi/writei/iomove and error */
	u.u_procp->p_sig |= resp->rp_sig;
	DUPRINT3(DB_SIGNAL,"replyuarg: rp_sig=%x,p_sig=%x\n",resp->rp_sig,u.u_procp->p_sig);
	switch (u.u_syscall)  {

	case DUSEEK:	
	{
		register struct a  { int fdes; off_t off; int sbase; } 
		*uap = (struct a *)u.u_ap;
		uap->off += resp->rp_rval;
		ret = TRUE;
		break;
	}
	case DUFCNTL:
		if (resp->rp_errno == 0)
			switch (resp->rp_rval) {
			case F_GETLK:
				if (copyout (resp->rp_data, resp->rp_bufptr,
					     resp->rp_count) < 0)
					u.u_error = EFAULT;
				break;
			case F_FREESP: {
				sndd_t sd;
				ip->i_size = resp->rp_isize;
				sd = (sndd_t)ip->i_fsptr;
				if (rcacheinit)
					rfinval(sd, -1, 0);
				break;
			}
			}
		ret = TRUE;
		break;
	case DUIOCTL:
		ret = TRUE;
		break;
	case DUFSTAT:
	{
		struct	stat	*statptr = (struct stat *) resp->rp_data;
		struct	gdp	*gp = (struct gdp *) ((queue_t *)((struct message *)
					in_bp->b_rptr)->m_queue)->q_ptr;
		extern struct gdp gdp[];

		hibyte (statptr->st_dev)  = ~(gp - gdp);
		if (resp->rp_errno == 0)
			if (copyout (resp->rp_data,
			  resp->rp_bufptr, resp->rp_count) < 0)
				resp->rp_errno = EFAULT;
			else
				prele(ip);
		break;
	}
	case DUFSTATFS:
		if (resp->rp_errno == 0)
			if (copyout(resp->rp_data, resp->rp_bufptr, resp->rp_count))
				u.u_error = EFAULT;
			else
				prele(ip);
		break;
	case DUGETDENTS:
	{
		register int i;
		  
		if (resp->rp_rval > 0)  {
			if (copyout (resp->rp_data,
			     resp->rp_bufptr,resp->rp_count) < 0)
				resp->rp_errno =  EFAULT;
		}
		if((u.u_rval1 = resp->rp_rval) > 0)
			u.u_offset = resp->rp_offset;
		u.u_ioch += (unsigned) u.u_rval1;
		ret = TRUE;
		break;
	}
	case DUUTSSYS:	/*  only ustat() gets this far  */
	{
		struct ustat  {
			daddr_t	f_tfree;
			ino_t	f_tinode;
			char	f_fname[6];
			char	f_fpack[6];
		};
		struct a { struct ustat *usb; int dev; int cmd; } *uap =
		  (struct a *) u.u_ap;

		if (resp->rp_errno == 0)
			if (copyout (resp->rp_data, resp->rp_bufptr,
			  sizeof (struct ustat)) < 0)
				resp->rp_errno = EFAULT;
			else
				free_sndd(sd);
		break;
	}
	case DUREAD:
		dinfo.oreadch += (u.u_count - resp->rp_rval);	/* ch's read by outgoing read's */
		if (ip->i_mntdev->m_rflags & MCACHE &&
		    resp->rp_cache & CACHE_ENABLE) {
			DUPRINT1(DB_CACHE, "duread: cache reenabled\n");
			sd->sd_stat |= SDCACHE;
		}
	case DUREADI:
		ip->i_size = resp->rp_isize;
		u.u_offset += u.u_count - resp->rp_rval;
		u.u_count = resp->rp_rval;
		ret = TRUE;
		break;
	case DUWRITE:
		dinfo.owritech += (u.u_count - resp->rp_rval);	/* ch's written by outgoing write's */
	case DUWRITEI:
	{
		register struct a {
			int	fdes;
			char	*cbuf;
			unsigned count;
		} *uap = (struct a *) u.u_ap;
		if (resp->rp_errno && (u.u_rcstat & U_RCACHE))
			/* invalidate cache buffers if write has error */
			rfinval(sd, off_to_blkoff(u.u_offset),
				ct_to_blkct(u.u_count, u.u_offset));
		u.u_offset += u.u_count - resp->rp_rval;
		u.u_count = resp->rp_rval;
			
		if (u.u_fmode&FAPPEND) {
			((struct file *)u.u_ofile[uap->fdes])->f_offset = resp->rp_isize;
			ip->i_size = resp->rp_isize + uap->count - u.u_count;
		}
		else
			ip->i_size = resp->rp_isize;
		ret = TRUE;
		break;
	}
	}  /*  end switch  */
	if (u.u_error || (u.u_error = resp->rp_errno))
		ret = TRUE;
	return (ret);
}
