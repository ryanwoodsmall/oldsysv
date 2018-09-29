/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sys3.c	10.13"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/ino.h"
#include "sys/iobuf.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/file.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/conf.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/ioctl.h"
#include "sys/var.h"
#include "sys/ttold.h"
#include "sys/open.h"
#include "sys/message.h"
#include "sys/debug.h"
#include "sys/utsname.h"

/*
 * the fstat system call.
 */
fstat()
{
	struct file *getf();
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	plock(ip = fp->f_inode);
	stat1(ip, uap->sb);
	prele(ip);
}

/*
 * the stat system call.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((ip = namei(upath, 0)) == NULL)
		return;
	stat1(ip, uap->sb);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
register struct inode *ip;
struct stat *ub;
{
	struct stat ds;

	if (ip->i_flag & (IACC|IUPD|ICHG))
		FS_IUPDAT(ip, &time, &time);
	/*
	 * first copy from inode table
	 */
	ds.st_dev = ip->i_dev;
	ds.st_ino = (ushort)ip->i_number;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = (dev_t)ip->i_rdev;
	ds.st_size = ip->i_size;
	/*
	 * Call fs dependent portion to 
	 * read the disk inode to obtain
	 * file modes and dates
	 */
	FS_STATF(ip, &ds);
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * the fstatfs system call.
 */
fstatfs()
{
	struct file *getf();
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		struct	statfs *sb;
		int	len;
		int	fstyp;
	} *uap = (struct a *)u.u_ap;

	if ((fp = getf(uap->fdes)) == NULL)
		return;
	plock(ip = fp->f_inode);
	statfs1(ip, uap->sb, uap->len, uap->fstyp);
	prele(ip);
}

/*
 * the statfs system call.
 */
statfs()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct	statfs *sb;
		int	len;
		int	fstyp;
	} *uap = (struct a *)u.u_ap;

	if ((ip = namei(upath, 0)) == NULL)
		return;
	statfs1(ip, uap->sb, uap->len, uap->fstyp);
	iput(ip);
}

/*
 * Common routine for fstatfs and statfs.
 */
statfs1(ip, ub, len, fstyp)
register struct inode *ip;
struct statfs *ub;
int len, fstyp;
{
	struct statfs ds;

	if (len < 0 || len > sizeof(ds) || fstyp < 0 || fstyp >= nfstyp) {
		u.u_error = EINVAL;
		return;
	}
	if (fstyp && ip->i_ftype != IFCHR && ip->i_ftype != IFBLK) {
		u.u_error = EINVAL;
		return;
	}
	/*
	 * Get generic superblock from fs-dependent code.
	 */
	(*fstypsw[fstyp ? fstyp : ip->i_fstyp].fs_statfs)(ip, &ds, fstyp);
	if (u.u_error == 0 && copyout((caddr_t)&ds, (caddr_t)ub, len) < 0)
		u.u_error = EFAULT;
}

/*
 * the dup system call.
 */
dup()
{
	struct file *getf();
	register struct file *fp;
	register i;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if ((i = ufalloc(0)) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the file control system call.
 */
fcntl()
{
	struct file *getf();
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register i;
	off_t offset;
	int flag;
	
	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	flag = fp->f_flag;
	offset = fp->f_offset;

	switch (uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i < 0 || i >= v.v_nofiles) {
			u.u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		u.u_ofile[i] = fp;
		fp->f_count++;
		break;

	case F_GETFD:
		u.u_rval1 = u.u_pofile[uap->fdes];
		break;

	case F_SETFD:
		u.u_pofile[uap->fdes] = uap->arg;
		break;

	case F_GETFL:
		u.u_rval1 = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		FS_FCNTL(fp->f_inode, F_CHKFL, uap->arg, flag, offset);
		if (u.u_error)
			return;
		uap->arg &= FMASK;
		fp->f_flag &= (FREAD|FWRITE);
		fp->f_flag |= (uap->arg-FOPEN) & ~(FREAD|FWRITE);
		break;

	default:
		FS_FCNTL(fp->f_inode, uap->cmd, uap->arg, flag, offset);
		break;
	}
}

/*
 * character special i/o control
 */
ioctl()
{
	struct file *getf();
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	FS_IOCTL(fp->f_inode, uap->cmd, uap->arg, fp->f_flag);
}

/*
 * old stty and gtty
 */
stty()
{
	register struct a {
		int	fdes;
		int	arg;
		int	narg;
	} *uap;

	uap = (struct a *)u.u_ap;
	uap->narg = uap->arg;
	uap->arg = TIOCSETP;
	u.u_syscall = DUIOCTL;
	ioctl();
}

gtty()
{
	register struct a {
		int	fdes;
		int	arg;
		int	narg;
	} *uap;

	uap = (struct a *)u.u_ap;
	uap->narg = uap->arg;
	uap->arg = TIOCGETP;
	u.u_syscall = DUIOCTL;
	ioctl();
}

/*
 * the mount system call.
 */
smount()
{
	register struct inode *bip, *ip = NULL;
	register struct mount *mp, *tmp;
	register short fstyp;
	register struct a {
		char	*fspec;
		char	*freg;
		int	flags;
		int	fstyp;
	} *uap;
	register dev_t dev;
	int special;

	if (!suser())
		return;
	uap = (struct a *)u.u_ap;
	if ((bip = namei(upath, 0)) == NULL)
		return;
	dev = bip->i_rdev;
	special = (bip->i_ftype == IFBLK || bip->i_ftype == IFCHR);
	tmp = NULL;
	for (mp = mount; mp < (struct mount *)v.ve_mount; mp++) {
		if (mp->m_flags == MFREE) {
			if (tmp == NULL) {	/* first empty slot */
				tmp = mp;
				tmp->m_flags = MINTER;
			}
		} else if (special && dev == mp->m_dev) {
			/* already busy */
			mp = NULL;
			goto out;
		}
	}
	if ((mp = tmp) == NULL)	/* no free slots */
		goto out;

	mp->m_dev = dev;
	mp->m_bcount = 0;
	u.u_mntindx = mp - mount;
	u.u_syscall = DULBMOUNT;

	u.u_dirp = (caddr_t)uap->freg;
	prele(bip);	/* unlock in case ip == bip */
	if ((ip = namei(upath, 0)) == NULL) {
		plock(bip);
		goto out;
	} else if (ip == bip) {
		ip->i_count--;
		if (special) {
			u.u_error = ENOTDIR;
			goto out;
		}
	} else
		plock(bip);
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		goto out;
	}
	if (ip->i_count != 1 || (ip->i_flag & IISROOT))
		goto out;
	/*
	 * Backward compatibility: require the user program to
	 * supply a flag indicating a new-style mount, otherwise
	 * assume the fstyp of the root file system.
	 */
	if ((uap->flags & MS_FSS) == 0)
		fstyp = mount[0].m_fstyp;
	else if ((fstyp = uap->fstyp) <= 0 || fstyp >= nfstyp) {
		u.u_error =  EINVAL;
		goto out;
	}
	(*fstypsw[fstyp].fs_mount)(bip, mp, uap->flags);
	if (u.u_error)
		goto out;
	mp->m_rflags = 0;
	ip->i_flag |= IMOUNT;
	ip->i_mnton = mp;
	mp->m_inodp = ip;
	if (ip != bip)
		iput(bip);
	prele(ip);
	mp->m_flags &= ~MINTER;
	mp->m_flags |= MINUSE;
	return;

out:	/* cleanup on error */
	if (ip != bip)
		iput(bip);
	if (ip != NULL) 
		iput(ip);
	if (mp != NULL)
		mp->m_flags = MFREE;
	if (u.u_error == 0)
		u.u_error = EBUSY;
}

/*
 * the umount system call.
 */
sumount()
{
	register struct inode *bip, *ip;
	register struct mount *mp;
	register struct a {
		char	*file;	/* directory or block-special */
	} *uap;
	register dev_t dev;
	register special = 0;

	if (!suser())
		return;
	uap = (struct a *)u.u_ap;
	if ((bip = namei(upath, 0)) == NULL)
		return;
	if (bip->i_ftype != IFDIR) {
		special = 1;
		dev = bip->i_rdev;
	}
	for (mp = mount; mp < (struct mount *)v.ve_mount; mp++)
		if ((mp->m_flags & MINUSE) && (special ?
	    	  dev == mp->m_dev : bip == mp->m_mount))
			goto found;

	u.u_error = EINVAL;
	iput(bip);
	goto out;

found:
	iput(bip);
	mp->m_flags &= ~MINUSE;
	mp->m_flags |= MINTER;
	(*fstypsw[mp->m_fstyp].fs_umount)(mp);
	if (u.u_error)
		goto out;
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	ip->i_mnton = NULL;
	plock(ip);
	iput(ip);
	mp->m_inodp = NULL;

out:
	if (u.u_error) {
		mp->m_flags &= ~MINTER;
		mp->m_flags |= MINUSE;
	} else
		mp->m_flags = MFREE;
}

/*
 * mount root file system
 * iflg is 1 if called from iinit, 0 if from uadmin/remount
 */
srmountfun(iflg)
{
	s5rmount(iflg);
}

/*
 * Unmount root file system
 * (called from uadmin sys call)
 */
srumountfun()
{
	s5rumount();
}
