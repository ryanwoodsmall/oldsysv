/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sys2.c	10.20"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/nami.h"
#include "sys/sysinfo.h"
#include "sys/fcntl.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/mount.h"
#include "sys/message.h"
#include "sys/debug.h"
#include "sys/var.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/poll.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

/*
 * read system call
 */
read()
{
	sysinfo.sysread++;
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	sysinfo.syswrite++;
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi or writei code.
 */
rdwr(mode)
register mode;
{
	register struct file	*fp;
	register struct inode	*ip;
	extern file_t		*getf();
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	}			*uap;
	int			type;
	int			bcount;
	register uint		ulimit;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if ((fp->f_flag & mode) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (caddr_t)uap->cbuf;
	ip = fp->f_inode;
	type = ip->i_ftype;

	u.u_count = uap->count;
	u.u_segflg = 0;
	u.u_fmode = fp->f_flag;
	if (type == IFREG || type == IFDIR) {
		plock(ip);
		if ((u.u_fmode&FAPPEND) && (mode == FWRITE))
			fp->f_offset = ip->i_size;

		/*	Make sure that the user can read all the way up
		**	to the ulimit value.
		*/

		ulimit = (uint)(u.u_limit << SCTRSHFT) - fp->f_offset;
		if (type == IFREG  &&  mode == FWRITE
		   && ulimit < uap->count  &&  ulimit > 0)
			uap->count = u.u_count = ulimit;

	} else if (type == IFIFO) {
		plock(ip);
		fp->f_offset = 0;
	}
	u.u_offset = fp->f_offset;
	if (mode == FREAD) {
		bcount = u.u_ior;
		FS_READI(ip);
	} else {
		bcount = u.u_iow;
		FS_WRITEI(ip);
	}
	if (u.u_error) {
		if (type == IFREG || type == IFDIR || type == IFIFO)
			prele(ip);
		return;
	}
	if (type == IFREG) {
		/* If synchronous write, update inode now. */
		if ((u.u_fmode & FSYNC) && (mode == FWRITE)) {
			ip->i_flag |= ISYN;
			FS_IUPDAT(ip, &time, &time);
		}
		prele(ip);
	} else if (type == IFDIR || type == IFIFO)
		prele(ip);

	/*  **************************************************** */
	/*  Used by 3bnet to prevent f_offset from being changed */

	if ((fp->f_flag & FNET) == 0)
		fp->f_offset += uap->count - u.u_count;
	/*  **************************************************** */

	u.u_rval1 = uap->count-u.u_count;
	u.u_ioch += (unsigned)u.u_rval1;
	if (mode == FREAD) {
		sysinfo.readch += (unsigned)u.u_rval1;
		fp->f_inode->i_mntdev->m_bcount += u.u_ior - bcount;
	} else {
		sysinfo.writech += (unsigned)u.u_rval1;
		fp->f_inode->i_mntdev->m_bcount += u.u_iow - bcount;
	}
}

/*
 * getmsg system call
 */
getmsg()
{
	msgio(FREAD);
}

/*
 * putmsg system call
 */
putmsg()
{
	msgio(FWRITE);
}

/*
 * common code for recv and send calls:
 * check permissions, copy in args, preliminary setup,
 * and switch to appropriate stream routine
 */
msgio(mode)
register mode;
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int		fdes;
		struct strbuf	*ctl;
		struct strbuf	*data;
		int		flags;
	} *uap;
	struct strbuf msgctl, msgdata;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if (!(fp->f_flag&mode)) {
		u.u_error = EBADF;
		return;
	}
	u.u_segflg = 0;
	u.u_fmode = fp->f_flag;
	ip = fp->f_inode;
	if ((ip->i_ftype != IFCHR) || (ip->i_sptr == NULL)) {
		u.u_error = ENOSTR;
		return;
	}
	if (uap->ctl) {
		if (copyin((caddr_t)uap->ctl, (caddr_t)&msgctl, 
		    sizeof(struct strbuf))) {
			u.u_error = EFAULT;
			return;
		}
	}
	if (uap->data) {
		if (copyin((caddr_t)uap->data, (caddr_t)&msgdata,
		   sizeof(struct strbuf))) {
			u.u_error = EFAULT;
			return;
		}
	}
	if (mode == FREAD) {
		if (!uap->ctl)
			msgctl.maxlen = -1;
		if (!uap->data)
			msgdata.maxlen = -1;
		u.u_rval1 = strgetmsg(ip, &msgctl, &msgdata, uap->flags);
		if (u.u_error)
			return;
		if ((uap->ctl && copyout((caddr_t)&msgctl, (caddr_t)uap->ctl, sizeof(struct strbuf))) ||
		    (uap->data && copyout((caddr_t)&msgdata, (caddr_t)uap->data, sizeof(struct strbuf)))) {
			u.u_error = EFAULT;
			return;
		}
		return;
	}
	/*
	 * FWRITE case 
	 */
	if (!uap->ctl)
		msgctl.len = -1;
	if (!uap->data)
		msgdata.len = -1;
	strputmsg(ip, &msgctl, &msgdata, uap->flags);
}

/*
 * Poll system call
 */

int pollwait;

poll()
{
	register struct uap  {
		struct	pollfd *fdp;
		unsigned long nfds;
		long	timo;
	} *uap = (struct uap *)u.u_ap;
	struct pollfd pollfd[NPOLLFILE];
	caddr_t fdp;
	register fdcnt = 0;
	register int i, j, s;
	extern time_t lbolt;
	time_t t;
	int rem;
	struct file *fp;
	int polltime();
	struct strevent *timeproc;
	int id, size;
	int mark;

	if (uap->nfds < 0 || uap->nfds > v.v_nofiles) {
		u.u_error = EINVAL;
		return;
	}
	t = lbolt;

	/*
	 * retry scan of fds until an event is found or until the
	 * timeout is reached.
	 */
retry:		

	/*
	 * Polling the fds is a relatively long process.  Set up the SPOLL
	 * flag so that we can see if something happened
	 * to an fd after we checked it but before we go to sleep.
	 */
	u.u_procp->p_flag |= SPOLL;

	/*
	 * Check fd's for specified events.  
	 * Read in pollfd records in blocks of NPOLLFILE.  Test each fd in the block
	 * and store the result of the test in the event field of the in-core
	 * record.  After a block of fds is finished, write the result out
	 * to the user.  Note that if no event is found, the whole procedure
	 * will be repeated after awakenening from the sleep (subject to timeout).
	 */

	mark = uap->nfds;
	size = 0;
	fdp = (caddr_t)uap->fdp;
	for (i = 0; i < uap->nfds; i++) {
		j = i % NPOLLFILE;
		/*
		 * If we have looped back around to the base of pollfd,
		 * write out the results of the strpoll calls kept in pollfd
		 * to the user fdp.  Read in the next batch of fds to check.
		 */
		if (!j) {
			if (i > 0) {
				ASSERT(size == NPOLLFILE*sizeof(struct pollfd));
				if (copyout(pollfd, fdp, size)) {
					u.u_error = EFAULT;
					return;
				}
				fdp += size;
			}
			size = min(uap->nfds - i, NPOLLFILE) * sizeof(struct pollfd);
			if (copyin((caddr_t)uap->fdp, (caddr_t)pollfd, size)) {
				u.u_error = EFAULT;
				return;
			}
		}

		if (pollfd[j].fd < 0) 
			pollfd[j].revents = 0;
		else if ( (pollfd[j].fd >= v.v_nofiles) ||
			  !(fp = u.u_ofile[pollfd[j].fd]) || 
			  (fp->f_inode->i_ftype != IFCHR) ||
			  !fp->f_inode->i_sptr) 
			pollfd[j].revents = POLLNVAL;
		else {
			pollfd[j].revents = strpoll(fp->f_inode->i_sptr, 
						     pollfd[j].events, fdcnt);
			if (u.u_error) {
				if (!fdcnt) mark = i;
				goto pollout;
			}
		}
		if (pollfd[j].revents && !fdcnt++) mark = i;
	}

	/*
	 * Poll of fds completed.  
	 * Copy out the last batch of events.  If the poll was successful, 
	 * return fdcnt to user.
	 */
	u.u_rval1 = fdcnt;
	if (copyout((caddr_t)pollfd, fdp, size)) {
		u.u_error = EFAULT; 
		return;
	}
	if (fdcnt) 
		goto pollout;

	/*
	 * If you get here, the poll of fds was unsuccessful.
	 * First make sure your timeout hasn't been reached.
	 * If not then sleep and wait until some fd becomes
	 * readable, writeable, or gets an exception.
	 */
	rem = ( (uap->timo < 0) ? 1 : (uap->timo - ((lbolt - t)*1000)/HZ) );
	if (rem <= 0)
		goto pollout;

	s = spl6();
	/*
	 * If anything has happened on an fd since it was checked, it will
	 * have turned off SPOLL.  Check this and rescan if so.
	 */
	if (!(u.u_procp->p_flag & SPOLL)) {
		splx(s);
		goto retry;
	}
	u.u_procp->p_flag &= ~SPOLL;

	if (!(timeproc = sealloc(SE_SLEEP))) {
		splx(s);
		u.u_error = EAGAIN;
		goto pollout;
	}
	timeproc->se_procp = u.u_procp;
	if (uap->timo > 0) 
		id = timeout(polltime, timeproc, (rem*HZ+999)/1000);

	/*
	 * The sleep will usually be awakened either by this poll's timeout 
	 * (which will have nulled timeproc), or by the strwakepoll function 
	 * called from a stream head.
	 */
	if (sleep((caddr_t)&pollwait, (PZERO+1)|PCATCH)) {
		if (uap->timo > 0)
			untimeout(id);
		splx(s);
		u.u_error = EINTR;
		sefree(timeproc);
		goto pollout;
	}
	splx(s);
	if (uap->timo > 0)
		untimeout(id);

	/*
	 * If timeproc is not NULL, you were probably awakened because a
	 * write queue emptied, a read queue got data, or an exception
	 * condition occurred.  If so go back up and poll fds again.
	 * Otherwise, you've timed out so you will fall thru and return.
	 */
	if (timeproc->se_procp) {
		sefree(timeproc);
		goto retry;
	}
	sefree(timeproc);

pollout:

	/*
	 * Poll general cleanup code. Go back to all of the streams 
	 * before the mark and reset the wakeup mechanisms that were 
	 * set up during the poll.  
	 */
	u.u_procp->p_flag &= ~SPOLL;
	fdp = (caddr_t)uap->fdp;
	for (i = 0; i < mark; i++) {
		j = i % NPOLLFILE;
		/*
		 * Read in next block of pollfds.  If the total number of pollfds
		 * is less than NPOLLFILE, don't bother because the pollfds of 
		 * interest are still in the pollfd[] array.
		 */
		if (!j && (uap->nfds > NPOLLFILE)) {
			size = min(uap->nfds - i, NPOLLFILE) * sizeof(struct pollfd);
			if (copyin((caddr_t)uap->fdp, (caddr_t)pollfd, size)) {
				u.u_error = EFAULT;
				return;
			}
			fdp += size;
		}

		if (pollfd[j].fd < 0)
			continue;
		if ((fp = getf(pollfd[j].fd)) == NULL)
			continue;
		if (fp->f_inode->i_sptr == NULL)
			continue;
		pollreset(fp->f_inode->i_sptr);
	}
}

/*
 * Removes all event cells that refer to the current process in the
 * given stream's poll list.
 */
pollreset(stp)
register struct stdata *stp;
{
	register struct strevent *psep, *sep, *tmp;
	
	sep = stp->sd_pollist;
	psep = NULL;
	while (sep) {
		tmp = sep->se_next;
		if (sep->se_procp == u.u_procp) {
			if (psep)
				psep->se_next = tmp;
			else
				stp->sd_pollist = tmp;
			sefree(sep);
		}
		sep = tmp;
	}
	/*
	 * Recalculate pollflags
	 */
	stp->sd_pollflags = 0;
	for (sep = stp->sd_pollist; sep; sep = sep->se_next)
		stp->sd_pollflags |= sep->se_events;
}

/*
 * This function is placed in the callout table to time out a process
 * waiting on poll.  If the poll completes, this function is removed
 * from the table.  Its argument is a pointer to a variable which holds
 * the process table pointer for the process to be awakened.  This
 * variable is nulled to indicate that polltime ran.
 */
polltime(timeproc)
struct strevent *timeproc;
{
	register struct proc *p = timeproc->se_procp;

	if (p->p_wchan == (caddr_t)&pollwait) {
		setrun(p);
		timeproc->se_procp = NULL;
	}
}

/*
 * open system call
 */
open()
{
	register struct a {
		char	*fname;
		int	mode;
		int	crtmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(uap->mode-FOPEN, uap->crtmode);
}

/*
 * creat system call
 */
creat()
{
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(FWRITE|FCREAT|FTRUNC, uap->fmode);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
copen(mode, arg)
register mode;
{
	register struct inode *ip;
	struct argnamei nmarg;

	if ((mode&(FREAD|FWRITE)) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if (ufalloc(0) < 0)	/* REMOTE - don't do anything if no room */
		return;
	if (mode&FCREAT) {
		nmarg.cmd = (mode&FEXCL) ? NI_XCREAT : NI_CREAT;
		nmarg.mode = arg & MODEMSK;
		nmarg.ftype = 0;
		nmarg.idev = -1;
		if ((ip = namei(upath, &nmarg)) == NULL) 
			return;
		if (nmarg.rcode == FSN_FOUND)
			mode &= ~FCREAT;
		else
			mode &= ~FTRUNC;
	} else if ((ip = namei(upath, 0)) == NULL)
			return;
	copen1(ip, mode);
}

copen1(ip, mode)
register struct inode *ip;
register mode;
{
	register struct file *fp;
	int i;

	if (!(mode&FCREAT)) {
		if (mode&FREAD)
			FS_ACCESS(ip, IREAD);
		if (mode&(FWRITE|FTRUNC)) {
			FS_ACCESS(ip, IWRITE);
			if (ip->i_ftype == IFDIR)
				u.u_error = EISDIR;
			else if ((mode&FTRUNC)
				 && (!FS_ACCESS(ip, IMNDLCK))
				 && (ip->i_filocks != NULL))
				u.u_error = EAGAIN;
		}
	}
	if (u.u_error || (fp = falloc(ip, mode&FMASK)) == NULL) {
		iput(ip);
		return;
	}
	if (mode&FTRUNC)
		FS_ITRUNC(ip);
	prele(ip);
	i = u.u_rval1;
	if (setjmp(u.u_qsav)) {	/* catch half-opens */
		if (u.u_error == 0)
			u.u_error = EINTR;
		u.u_ofile[i] = NULL;
		closef(fp);
	} else {
		FS_OPENI(ip, mode);
		if (u.u_error == 0)
			return;
		u.u_ofile[i] = NULL;
		unfalloc(fp);
		plock(ip);
		iput(ip);
	}
}

/*
 * close system call
 */
close()
{
	register file_t *fp;
	extern file_t	*getf();
	register struct a {
		int	fdes;
	}		*uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	u.u_ofile[uap->fdes] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	register struct file *fp;
	register struct inode *ip;
	off_t offset;
	struct argnotify noarg;
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;

	/*  *********************************************** */
	/*  ************ Used by 3bnet ******************** */
	if (fp->f_flag & FNET) {
		u.u_error = ESPIPE;
		psignal(u.u_procp, SIGSYS);
		return;
	}
	/*  *********************************************** */

	ip = fp->f_inode;
	if (ip->i_ftype == IFIFO) {
		u.u_error = ESPIPE;
		return;
	}
	if (uap->sbase == 1)
		uap->off += fp->f_offset;
	else if (uap->sbase == 2)
		uap->off += fp->f_inode->i_size;
	else if (uap->sbase != 0) {
		u.u_error = EINVAL;
		psignal(u.u_procp, SIGSYS);
		return;
	}
	if (fsinfo[ip->i_fstyp].fs_notify & NO_SEEK) {
		noarg.cmd = NO_SEEK;
		noarg.data1 = uap->off;
		noarg.data2 = uap->sbase;
		offset = FS_NOTIFY(ip, &noarg);
		if (u.u_error)
			return;
	} else
		offset = uap->off;
	fp->f_offset = offset;
	u.u_roff = offset;
}

/*
 * link system call
 */
link()
{
	register struct inode *ip;
	struct argnamei nmarg;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;

	u.u_pdir = NULL;
	uap = (struct a *)u.u_ap;
	if ((ip = namei(upath, 0)) == NULL)
		return;
	u.u_pdir = ip;
	if (ip->i_ftype == IFDIR && !suser())
		goto out;

	if (server())
		return;
	prele(ip);
	u.u_dirp = (caddr_t)uap->linkname;
	nmarg.cmd = NI_LINK;
	nmarg.idev =  ip->i_dev;
	nmarg.ino =  ip->i_number;
	(void)namei(upath, &nmarg);
	plock(ip);
out:
	iput(ip);
}


/*
 * mknod system call
 */
mknod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	if ((uap->fmode&IFMT) != IFIFO && !suser())
		return;
	nmarg.cmd = NI_MKNOD;
	nmarg.mode = uap->fmode&MODEMSK;
	nmarg.ftype = uap->fmode&IFMT;
	nmarg.idev = (dev_t)uap->dev;
	if ((ip = namei(upath, &nmarg)) != NULL)
		iput(ip);
}

/*
 * access system call
 */
saccess()
{
	register svuid, svgid;
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	u.u_ruid = svuid;
	u.u_rgid = svgid;

	if ((ip = namei(upath, 0)) != NULL) {
		if (uap->fmode&(IREAD>>6))
			FS_ACCESS(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			FS_ACCESS(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			FS_ACCESS(ip, IEXEC);
		iput(ip);
	}
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	u.u_ruid = svuid;
	u.u_rgid = svgid;
}

/*
 * mkdir system call
 */
mkdir()
{
	register struct a {
		char *fname;
		int fmode;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	nmarg.cmd = NI_MKDIR;
	nmarg.mode = (uap->fmode & PERMMSK);
	(void) namei(upath, &nmarg);
}

/*
 * rmdir system call
 */
rmdir()
{	struct a {
		char *fname;
	};
	struct argnamei nmarg;

	nmarg.cmd = NI_RMDIR;
	(void) namei(upath, &nmarg);
}

/*
 * getdents system call
 */
getdents()
{
	register struct inode *ip;
	register struct file *fp;
	int num;

	extern file_t	*getf();
	struct a {
		int fd;
		char *buf;
		int nbytes;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fd)) == NULL)
		return;
	ip = fp->f_inode;
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		return;
	}
	u.u_offset = fp->f_offset;
	num = FS_GETDENTS(ip, uap->buf, uap->nbytes);
	if (num > 0)
		fp->f_offset = u.u_offset;
	u.u_rval1 = num;
}
