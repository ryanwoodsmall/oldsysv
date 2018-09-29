/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/fileop.c	10.17"

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
	register int	size = 0;	/*  for de_queue, received size		*/
	sndd_t	gift = NULL;		/* return value of de_queue */
	register int	ret;		/* return instead of longjmp */

	DUPRINT3(DB_SYSCALL, "remfileop: syscall = %d (%s)\n", 
			u.u_syscall, sysname(u.u_syscall));
	if (server()) {	/* if on server, this is multihop	*/
		u.u_error = EMULTIHOP;
		return(FAILURE);
	}
	sysinfo.fileop++;
	/* get a buffer for the request
	 */
	bp = alocbuf(sizeof (struct request), BPRI_LO);
	if (bp == NULL) {
		u.u_error = EINTR;
		return(FAILURE);
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
	if ((size = filluargs (bp, &gift, flag, offset)) == -1) {
		freemsg (bp);
		return(FAILURE);
	}
	dinfo.osyscall++;		/* outgoing system calls */

	/*  send the message and wait for the response  */
	if (rsc (sd, bp, sizeof (struct request) - DATASIZE + size,
             &in_bp, gift) == FAILURE)  {
		if (gift) free_sndd (gift);
		return(FAILURE);
	}

	/*  now take the arguments out of the message and put back  into
	 *  u area.
	 */
	resp = (struct response *)PTOMSG(in_bp->b_rptr);
	if (gift) free_sndd (gift);
	ret = replyuargs (resp,in_bp,ip);
	freemsg (in_bp);

	/* Normally longjmp back to trap.c/syscall - we's done.
	 * readi/writei and errors return.
	 */
	if (ret)
		return (SUCCESS);
	else  {
		u.u_rflags |= U_RSYS;
		longjmp (u.u_qsav, 1);
	}

	
}

/*  set values in the message based on the system call.
 *  These are the file descriptor system calls.
 *  The server recreates the file descriptor before calling the system call.
 */

static
filluargs (bp, gift, flag, offset)
mblk_t	*bp;
sndd_t	*gift;
register flag, offset;
{
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
	register int ret = FALSE;  /* TRUE for readi/writei/iomove and error */
	u.u_procp->p_sig |= resp->rp_sig;
	DUPRINT3(DB_SIGNAL,"replyuarg: rp_sig=%x,p_sig=%x\n",resp->rp_sig,u.u_procp->p_sig);
	switch (u.u_syscall)  {

	case DUSEEK:	
	{
		register struct a  { int fdes; off_t off; int sbase; } 
		*uap = (struct a *)u.u_ap;
		uap->off += resp->rp_rval;
	}
	case DUFCNTL:
		ret = TRUE;
	case DUIOCTL:
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
		if (resp->rp_errno == 0
		  && copyout(resp->rp_data, resp->rp_bufptr, resp->rp_count))
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
				free_sndd((sndd_t *)ip->i_fsptr);
		break;
	}
	case DUREAD:
		dinfo.oreadch += (u.u_count - resp->rp_rval);	/* ch's read by outgoing read's */
	case DUREADI:
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
		u.u_offset += u.u_count - resp->rp_rval;
		u.u_count = resp->rp_rval;
			
		if (u.u_fmode&FAPPEND) 
			((struct file *)u.u_ofile[uap->fdes])->f_offset = resp->rp_isize;
		ret = TRUE;
		break;
	}
	}  /*  end switch  */
	if (u.u_error || (u.u_error = resp->rp_errno))
		ret = TRUE;
	return (ret);
}
