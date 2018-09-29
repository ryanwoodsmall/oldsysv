/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)sys2.c	6.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/sysinfo.h"

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
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	int	type;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	if ((fp->f_flag&mode) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (caddr_t)uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	u.u_fmode = fp->f_flag;
	ip = fp->f_inode;
	type = ip->i_mode&IFMT;
	if (type==IFREG || type==IFDIR) {
		plock(ip);
		if ((u.u_fmode&FAPPEND) && (mode == FWRITE))
			fp->f_offset = ip->i_size;
	} else if (type == IFIFO) {
		plock(ip);
		fp->f_offset = 0;
	}
	u.u_offset = fp->f_offset;
	if (mode == FREAD)
		readi(ip);
	else
		writei(ip);
	if (type==IFREG || type==IFDIR || type==IFIFO)
		prele(ip);
	fp->f_offset += uap->count-u.u_count;
	u.u_rval1 = uap->count-u.u_count;
	u.u_ioch += (unsigned)u.u_rval1;
	if (mode == FREAD)
		sysinfo.readch += (unsigned)u.u_rval1;
	else
		sysinfo.writech += (unsigned)u.u_rval1;
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
	register struct file *fp;
	int i;

	if ((mode&(FREAD|FWRITE)) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if (mode&FCREAT) {
		ip = namei(uchar, 1);
		if (ip == NULL) {
			if (u.u_error)
				return;
			ip = maknode(arg&07777&(~ISVTX));
			if (ip == NULL)
				return;
			mode &= ~FTRUNC;
		} else {
			if (mode&FEXCL) {
				u.u_error = EEXIST;
				iput(ip);
				return;
			}
			mode &= ~FCREAT;
		}
	} else {
		ip = namei(uchar, 0);
		if (ip == NULL)
			return;
	}
	if (!(mode&FCREAT)) {
		if (mode&FREAD)
			access(ip, IREAD);
		if (mode&(FWRITE|FTRUNC)) {
			access(ip, IWRITE);
			if ((ip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if (u.u_error || (fp = falloc(ip, mode&FMASK)) == NULL) {
		iput(ip);
		return;
	}
	if (mode&FTRUNC)
		itrunc(ip);
	prele(ip);
	i = u.u_rval1;
	if (setjmp(u.u_qsav)) {	/* catch half-opens */
		if (u.u_error == 0)
			u.u_error = EINTR;
		u.u_ofile[i] = NULL;
		closef(fp);
	} else {
		openi(ip, mode);
		if (u.u_error == 0)
			return;
		u.u_ofile[i] = NULL;
		if ((--fp->f_count) <= 0) {
			fp->f_next = ffreelist;
			ffreelist = fp;
		}
		iput(ip);
	}
}

/*
 * close system call
 */
close()
{
	register struct file *fp;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
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
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	ip = fp->f_inode;
	if ((ip->i_mode&IFMT)==IFIFO) {
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
	if (uap->off < 0) {
		u.u_error = EINVAL;
		return;
	}
	fp->f_offset = uap->off;
	u.u_roff = uap->off;
}

/*
 * link system call
 */
link()
{
	register struct inode *ip, *xp;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0);
	if (ip == NULL)
		return;
	if (ip->i_nlink >= MAXLINK) {
		u.u_error = EMLINK;
		goto outn;
	}
	if ((ip->i_mode&IFMT)==IFDIR && !suser())
		goto outn;
	/*
	 * Unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone
	 * deletes the file in the meantime?)
	 * Nor can it be locked again later
	 * because then there will be deadly
	 * embraces.
	 * Update inode first for robustness.
	 */
	ip->i_nlink++;
	ip->i_flag |= ICHG|ISYN;
	iupdat(ip, &time, &time);
	prele(ip);
	u.u_dirp = (caddr_t)uap->linkname;
	xp = namei(uchar, 1);
	if (xp != NULL) {
		iput(xp);
		u.u_error = EEXIST;
		goto out;
	}
	if (u.u_error)
		goto out;
	if (u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
out:
	plock(ip);
	if (u.u_error) {
		ip->i_nlink--;
		ip->i_flag |= ICHG;
	}
outn:
	iput(ip);
	return;
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
		dev_t	dev;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((uap->fmode&IFMT) != IFIFO && !suser())
		return;
	ip = namei(uchar, 1);
	if (ip != NULL) {
		iput(ip);
		u.u_error = EEXIST;
		return;
	}
	if (u.u_error)
		return;
	ip = maknode(uap->fmode);
	if (ip == NULL)
		return;
	switch(ip->i_mode&IFMT) {
	case IFCHR:
	case IFBLK:
		ip->i_rdev = (dev_t)uap->dev;
		ip->i_flag |= ICHG;
	}

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
	ip = namei(uchar, 0);
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			access(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			access(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
