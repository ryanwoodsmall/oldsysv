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
/* @(#)fio.c	6.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/filsys.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/var.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.
 * Only task is to check range of the descriptor.
 */
struct file *
getf(f)
register int f;
{
	register struct file *fp;

	if (0 <= f && f < NOFILE) {
		fp = u.u_ofile[f];
		if (fp != NULL)
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 * Also make sure the pipe protocol does not constipate.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 * On the last close switch out to the device handler for
 * special files.  Note that the handler is called
 * on every open but only the last close.
 */
closef(fp)
register struct file *fp;
{
	register struct inode *ip;
	int flag, fmt;
	dev_t dev;
	register int (*cfunc)();

	if (fp == NULL)
		return;
	cleanlocks(fp);
	if ((unsigned)fp->f_count > 1) {
		fp->f_count--;
		return;
	}
	ip = fp->f_inode;
	plock(ip);
	flag = fp->f_flag;
	dev = (dev_t)ip->i_rdev;
	fmt = ip->i_mode&IFMT;
	fp->f_count = 0;
	fp->f_next = ffreelist;
	ffreelist = fp;
	switch(fmt) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		break;

	case IFBLK:
		cfunc = bdevsw[bmajor(dev)].d_close;
		break;

	case IFIFO:
		closep(ip, flag);

	default:
		iput(ip);
		return;
	}
	for (fp = file; fp < (struct file *)v.ve_file; fp++) {
		register struct inode *tip;

		if (fp->f_count) {
			tip = fp->f_inode;
			if (tip->i_rdev == dev &&
			  (tip->i_mode&IFMT) == fmt)
				goto out;
		}
	}
	if (setjmp(u.u_qsav))	/* catch half closes */
		goto out;
	if (fmt == IFBLK) {
		register struct mount *mp;

		for (mp = mount; mp < (struct mount *)v.ve_mount; mp++)
			if (mp->m_flags == MINUSE && mp->m_dev == dev)
				goto out;
		bflush(dev);
		(*cfunc)(minor(dev), flag);
		binval(dev);
	} else {
		prele(ip);
		(*cfunc)(minor(dev), flag);
	}
out:
	iput(ip);
}

/*
 * openi called to allow handler of special files to initialize and
 * validate before actual IO.
 */
openi(ip, flag)
register struct inode *ip;
{
	dev_t dev;
	register unsigned int maj;

	dev = (dev_t)ip->i_rdev;
	switch(ip->i_mode&IFMT) {

	case IFCHR:
		maj = major(dev);
		if (maj >= cdevcnt)
			goto bad;
		if (u.u_ttyp == NULL)
			u.u_ttyd = dev;
		(*cdevsw[maj].d_open)(minor(dev), flag);
		break;

	case IFBLK:
		maj = bmajor(dev);
		if (maj >= bdevcnt)
			goto bad;
		(*bdevsw[maj].d_open)(minor(dev), flag);
		break;

	case IFIFO:
		openp(ip, flag);
		break;
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the read-only status of the file
 * system is checked. Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select the owner/group/other fields.
 * The super user is granted all permissions.
 */
access(ip, mode)
register struct inode *ip;
{
	register m;

	m = mode;
	if (m == IWRITE) {
		if (getfs(ip->i_dev)->s_ronly) {
			u.u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&ITEXT)
			xrele(ip);
		if (ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if (u.u_uid == 0)
		return(0);
	if (u.u_uid != ip->i_uid) {
		m >>= 3;
		if (u.u_gid != ip->i_gid)
			m >>= 3;
	}
	if ((ip->i_mode&m) != 0)
		return(0);

	u.u_error = EACCES;
	return(1);
}

/*
 * Look up a pathname and test if the resultant inode is owned by the
 * current user. If not, try for super-user.
 * If permission is granted, return inode pointer.
 */
struct inode *
owner()
{
	register struct inode *ip;

	ip = namei(uchar, 0);
	if (ip == NULL)
		return(NULL);
	if (u.u_uid == ip->i_uid || suser())
		if (getfs(ip->i_dev)->s_ronly)
			u.u_error = EROFS;
	if (!u.u_error)
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the super user.
 */
suser()
{

	if (u.u_uid == 0) {
		u.u_acflag |= ASU;
		return(1);
	}
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(i)
register i;
{

	for(; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_rval1 = i;
			u.u_pofile[i] = 0;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor and a file structure.
 * Initialize the descriptor to point at the file structure.
 *
 * no file -- if there are no available file structures.
 */
struct file *
falloc(ip, flag)
struct inode *ip;
{
	register struct file *fp;
	register i;

	i = ufalloc(0);
	if (i < 0)
		return(NULL);
	if ((fp=ffreelist) == NULL) {
		printf("no file\n");
		syserr.fileovf++;
		u.u_error = ENFILE;
		return(NULL);
	}
	ffreelist = fp->f_next;
	u.u_ofile[i] = fp;
	fp->f_count++;
	fp->f_inode = ip;
	fp->f_flag = flag;
	fp->f_offset = 0;
	return(fp);
}

struct file *ffreelist;
finit()
{
	register struct file *fp;

	for (ffreelist = fp = &file[0]; fp < &file[v.v_file-1]; fp++)
		fp->f_next = fp+1;
}
