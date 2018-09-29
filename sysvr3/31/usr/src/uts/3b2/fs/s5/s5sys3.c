/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5sys3.c	10.15.1.2"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/ino.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
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
#include "sys/debug.h"

extern struct mount s5_1024mnt;
extern struct mount s5_512mnt;

s5statf(ip, st)
register struct inode *ip;
register struct stat *st;
{
	register struct buf *bp;
	register struct dinode *dp;
	register struct s5inode *s5ip;
	register int bsize;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	st->st_mode = s5ip->s5i_mode;		/* file modes */
	/*
	 * get the dates from the disk inode
	 */
	bsize = ip->i_mntdev->m_bsize;
	bp = bread(ip->i_dev, FsITOD(bsize, ip->i_number), FsBSIZE(bsize));
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(bsize, ip->i_number);
	st->st_atime = dp->di_atime;
	st->st_mtime = dp->di_mtime;
	st->st_ctime = dp->di_ctime;
	brelse(bp);
}

/* fcntl */

s5fcntl(ip, cmd, arg, flag, offset)
register struct inode *ip;
int cmd, arg, flag;
off_t offset;
{
	struct flock bf;
	int i;

	switch (cmd) {
	case F_GETLK:
		/* get record lock */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf, 0, flag, offset)) != 0)
			u.u_error = i;
		else if (copyout(&bf, (caddr_t)arg, sizeof bf))
			u.u_error = EFAULT;
		break;

	case F_SETLK:
		/* set record lock */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf, SETFLCK, flag, offset)) != 0) {
			/* The following if statement is to maintain
			** backward compatibility. Note: the return
			** value of reclock was not changed to
			** EAGAIN because rdwr() also calls reclock(),
			** and we want rdwr() to meet the standards.
			*/
			if ( i == EAGAIN )
				u.u_error = EACCES;
			else
				u.u_error = i;
		}
		break;

	case F_SETLKW:
		/* set record lock and wait */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf,
		  SETFLCK | SLPFLCK, flag, offset)) != 0) {
				u.u_error = i;
		}
		break;

	case F_CHKFL:
		break;

	case F_FREESP:
		/* free file storage space */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = convoff(ip, &bf, 0, offset)) != 0)
			u.u_error = i;
		else
			s5freesp(ip, &bf, flag);
		break;

	default:
		u.u_error = EINVAL;
	}
}

/* ioctl */
s5ioctl(ip, cmd, arg, flag)
register struct inode *ip;
int cmd, arg, flag;
{
	register dev_t dev;

	if (ip->i_ftype != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	dev = ip->i_rdev;
	if (cdevsw[major(dev)].d_str)
		strioctl(ip, cmd, arg, flag);
	else
		(*cdevsw[major(dev)].d_ioctl)(dev, cmd, arg, flag);
}

/*
 * the mount system call.
 */
/* ARGSUSED */
s5mount(bip, mp, flags, dataptr, datalen)
register struct inode *bip;
register struct mount *mp;
int flags;
char *dataptr;
int datalen;
{
	register dev_t dev;
	register struct filsys *fp;
	int rdonly;
	extern short s5fstyp;

	if (bip->i_ftype != IFBLK) {
		u.u_error = ENOTBLK;
		return;
	} else {
		dev = (dev_t)bip->i_rdev;
		if (bmajor(dev) >= bdevcnt) {
			u.u_error = ENXIO;
			return;
		}
	}
	dev = mp->m_dev;
	rdonly = (flags & MS_RDONLY);
	/* Why are FREAD and/or FWRITE passed to drivers...? */
 	(*bdevsw[bmajor(dev)].d_open)(dev, rdonly ? FREAD : FREAD|FWRITE,
 		OTYP_MNT);
#if 0
	(*bdevsw[bmajor(dev)].d_open)(dev, rdonly, OTYP_MNT);
#endif
	if (u.u_error)
		return;

	mp->m_bufp = (caddr_t)geteblk();
	fp = getfs(mp);
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	s5readi(bip);
	if (u.u_error)
		goto out;
	fp->s_fmod = 0;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = rdonly;
	if (fp->s_magic != FsMAGIC) {
#ifdef u3b2
		u.u_error = EINVAL;
		goto out;
#else
		fp->s_type = Fs1b;
#endif
	}
	if (rdonly)
		mp->m_flags |= MRDONLY;
	else {
		if ((fp->s_state + (long)fp->s_time) == FsOKAY) {
			fp->s_state = FsACTIVE;
			u.u_offset = SUPERBOFF;
			u.u_count = sizeof(struct filsys);
			u.u_base = (caddr_t)fp;
			u.u_segflg = 1;
			u.u_fmode = FWRITE|FSYNC;
			s5writei(bip);
			if (u.u_error) {
				u.u_error = EROFS;
				goto out;
			}
		} else {
			u.u_error = ENOSPC;
			goto out;
		}
	}

	/* determine fstyp */
	switch ((int) fp->s_type) {
#ifndef u3b2
	default:
#endif
	case Fs1b:
		mp->m_bsize = 512;
		break;
	case Fs2b:
		/*
		 * Invalidate buffer containing the superblock
		 * read by s5readi().
		 */
		binval(mp->m_dev);
		mp->m_bsize = 1024;
		break;
#ifdef u3b2
	default:
		/* 3B2 never had "old" 512-byte file systems */
		u.u_error = EINVAL;
		goto out;
#endif
	}
	mp->m_fstyp = s5fstyp;
	if (pipedev == mp->m_dev)
		pipedev = mp->m_dev;

	if (mp->m_mount = iget(mp, S5ROOTINO)) {
		mp->m_mount->i_flag |= IISROOT;
		prele(mp->m_mount);
	} else
		goto out;

	/*
	 * Initialize pipe device for this fs if this is the first 
	 * fs of this type that has been mounted.
	 */

	if (!fsinfo[mp->m_fstyp].fs_pipe)
		fsinfo[mp->m_fstyp].fs_pipe = mp;
	return;

out:	/* cleanup on error */
	binval(dev);
	(*bdevsw[bmajor(dev)].d_close)(dev, rdonly, OTYP_MNT);
	brelse(mp->m_bufp);
}

/*
 * the umount system call.
 */
s5umount(mp)
register struct mount *mp;
{
	dev_t dev;
	register struct inode *ip, *rip;
	register struct filsys *fp;
	struct inode in;
	struct s5inode s5in;

	dev = mp->m_dev;

	if (dev & Fs2BLK)
		cmn_err(CE_WARN, "s5umount dev has Fs2BLK set\n");

	xumount(mp);	/* remove unused sticky files from text table */

	/*
	 * If this device is the pipe device for this fs type:
	 * (1)	Return busy if the default pipefstyp is this fstyp.
	 *	If this wasn't done there would be no default pipsfstyp.
	 *	If busy is returned, the user can reassign the pipe
	 *	to another device.
	 * (2)	Clear the fs_pipe entry for this type if this device
	 *	was the pipe device for this fstyp.  By this time it has
	 *	been determined that no inode is active.
	 */

	if (fsinfo[mp->m_fstyp].fs_pipe == mp) {
		if (pipefstyp == mp->m_fstyp) {
			u.u_error = EBUSY;
			return;
		}
		fsinfo[mp->m_fstyp].fs_pipe = 0;
	}

	if (iflush(mp) < 0) {
		u.u_error = EBUSY;
		return;
	}
	/* Flush root inode to disk. */
	rip = mp->m_mount;
	ASSERT(rip != NULL);
	plock(rip);
	s5iupdat(rip, &time, &time);
	if (u.u_error) {
		prele(rip);
		return;
	}
	/*
	 * At this point there should be no active files on the
	 * file system, and the super block should not be locked.
	 * Break the connections.
	 */
	fp = getfs(mp);
	if (!fp->s_ronly) {
		bflush(dev);
		fp->s_time = time;
		fp->s_state = FsOKAY - (long)fp->s_time;
		ip = &in;
		ip->i_ftype = IFBLK;
		ip->i_rdev = mp->m_dev;
		ip->i_fstyp = mp->m_fstyp;
		ip->i_mntdev = mp;
		ip->i_fsptr = (int *)&s5in;
		s5in.s5i_mode = IFBLK;
		u.u_error = 0;
		u.u_offset = SUPERBOFF;
		u.u_count = sizeof(struct filsys);
		u.u_base = (caddr_t)fp;
		u.u_segflg = 1;
		u.u_fmode = FWRITE|FSYNC;
		s5writei(ip);
		u.u_error = 0;
	}
	(*bdevsw[bmajor(dev)].d_close)(dev, 0, OTYP_MNT);
	if (u.u_error) {
		prele(rip);
		return;
	}
	punmount(mp);
	binval(dev);
	brelse(mp->m_bufp);
	mp->m_bufp = NULL;
	iput(rip);
	iunhash(rip);
	mp->m_mount = NULL;
	/*
	 * If any error did occur since calling the close routine
	 * we have to ignore it since we've already disposed of the
	 * inode.  Otherwise we'd be left with a mount-table entry
	 * in an inconsistent state.
	 */
	u.u_error = 0;
}

/*
 * mount root file system
 * iflg is 1 if called from iinit, 0 if from uadmin/remount
 */
s5rmount(iflg)
{
	register struct mount	*mp = &mount[0];
	register struct filsys	*fp;
	struct inode iinode;
	struct s5inode s5in;
	extern short s5fstyp;
	register struct inode *ip;

	if (iflg) {	/* initial call from iinit */
		mp->m_bufp = (caddr_t)geteblk();
		fp = getfs(mp);
	} else {		/* for remount */
		binval(mp->m_dev);
		fp = getfs(mp);
		if (fp->s_state == FsACTIVE) {
			u.u_error = EINVAL;
			return;
		}
	}
	mp->m_flags = MINUSE;
	mp->m_dev = rootdev;
	mp->m_bsize = 1024; /* assume 1024 byte block fs */
	ip = &iinode;
	ip->i_ftype = IFBLK;
	ip->i_rdev = rootdev;
	ip->i_fsptr = (int *)&s5in;
	ip->i_mntdev = mp;
	s5in.s5i_mode = IFBLK;
	u.u_error = 0;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	s5readi(ip);
	if (u.u_error)
		cmn_err(CE_PANIC, "srmount - cannot mount root");
	fp->s_fmod = 0;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = 0;
	if (fp->s_magic != FsMAGIC)
		cmn_err(CE_PANIC, "srmount - not a valid root");

	if ((fp->s_state + (long)fp->s_time) == FsOKAY)
		fp->s_state = FsACTIVE;
	else
		fp->s_state = FsBAD;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	u.u_fmode = FWRITE|FSYNC;
	s5writei(ip);
	if (u.u_error) {
		fp->s_state = FsBAD;
		u.u_error = 0;
	}
	/* determine fstyp */
	switch ((int) fp->s_type) {
	default:
	case Fs1b:
		mp->m_bsize = 512;
		break;
	case Fs2b:
		/*
		 * Invalidate buffer containing the superblock
		 * read by s5readi().
		 */
		binval(mp->m_dev);
		mp->m_bsize = 1024;
		break;
	}
	mp->m_fstyp = s5fstyp;
	rootdev = mp->m_dev;
	/* Initialize pipe device for this fs. */
	fsinfo[mp->m_fstyp].fs_pipe = mp;
	/* Initialize pipefstyp to that of the default root. */
	pipefstyp = mp->m_fstyp;
	if (pipedev == rootdev)
		pipedev = rootdev;
	if (iflg) {
		clkset(fp->s_time);
		/* 
		 * get root inode for proc 0 and others
		 */
		rootdir = iget(&mount[0], S5ROOTINO);
		rootdir->i_flag |= IISROOT;
		prele(rootdir);
		u.u_cdir = iget(&mount[0], S5ROOTINO);
		prele(u.u_cdir);
		u.u_rdir = NULL;
	}
	mount[0].m_mount = rootdir;
}

s5rumount()
{
	register struct mount *mp = &mount[0];
	register struct filsys *fp;
	struct s5inode s5in;
	struct inode iinode;
	register struct inode *ip;

	fp = getfs(mp);
	if (fp->s_state == FsACTIVE) {
		ip = &iinode;
		ip->i_ftype = IFBLK;
		ip->i_rdev = rootdev;
		ip->i_fsptr = (int *)&s5in;
		ip->i_mntdev = mp;
		s5in.s5i_mode = IFBLK;
		fp->s_time = time;
		fp->s_state = FsOKAY - (long)fp->s_time;
		u.u_error = 0;
		u.u_offset = SUPERBOFF;
		u.u_count = sizeof(struct filsys);
		u.u_base = (caddr_t)fp;
		u.u_segflg = 1;
		u.u_fmode = FWRITE|FSYNC;
		s5writei(ip);
		u.u_error = 0;
	}
	bdwait();
}

s5statfs(ip, sp, ufstyp)
register struct inode *ip;
register struct statfs *sp;
int  ufstyp;
{
	register struct filsys *fp;
	register struct buf *bp;
	struct inode in;
	struct s5inode s5in;
	extern short s5fstyp;
	dev_t dev;

	if (ufstyp) {
		/*
		 * File system not mounted.  The inode pointer
		 * refers to a device from which the superblock
		 * must be read.
		 */
		dev = ip->i_rdev;
		if (ip->i_ftype != IFBLK) {
			/* Character devices not supported for now. */
			u.u_error = EINVAL;
			return;
		}
		(*bdevsw[bmajor(dev)].d_open)(dev, 0, OTYP_LYR);
		if (u.u_error)
			return;
		bp = geteblk();
		fp = (struct filsys *)bp->b_un.b_addr;
		in.i_ftype = IFBLK;
		in.i_rdev = dev;
		in.i_fstyp = ufstyp;
		in.i_fsptr = (int *)&s5in;
		in.i_mntdev = &s5_1024mnt;
		s5in.s5i_mode = IFBLK;
		u.u_offset = SUPERBOFF;
		u.u_count = sizeof(struct filsys);
		u.u_base = (caddr_t)fp;
		u.u_segflg = 1;
		s5readi(&in);
		if (u.u_error)
			goto out;
	} else
		fp = getfs(ip->i_mntdev);
	if (fp->s_magic != FsMAGIC) {
		u.u_error = EINVAL;
		goto out;
	}
	if (fp->s_type == Fs2b) {
		sp->f_bsize = 1024;
	} else if (fp->s_type == Fs1b) {
		sp->f_bsize = 512;
	} else {
		u.u_error = EINVAL;
		goto out;
	}
	sp->f_fstyp = s5fstyp;
	sp->f_frsize = 0;
	sp->f_blocks = FsLTOP(sp->f_bsize, fp->s_fsize);
	sp->f_bfree = FsLTOP(sp->f_bsize, fp->s_tfree);
	sp->f_files = (fp->s_isize - 2) * FsINOPB(sp->f_bsize);
	sp->f_ffree = fp->s_tinode;
	bcopy(fp->s_fname, sp->f_fname, sizeof(sp->f_fname));
	bcopy(fp->s_fpack, sp->f_fpack, sizeof(sp->f_fpack));
out:
	if (ufstyp) {
		(*bdevsw[bmajor(dev)].d_close)(dev, 0, OTYP_LYR);
		brelse(bp);
	}
}

#define NIADDR	3		/* number of indirect block pointers */
#define NDADDR	(NADDR-NIADDR)	/* number of direct block pointers */
#define IB(i)	(NDADDR + (i))	/* index of i'th indirect block ptr */
#define SINGLE	0		/* single indirect block ptr */
#define DOUBLE	1		/* double indirect block ptr */
#define TRIPLE	2		/* triple indirect block ptr */

/*
 * Free storage space associated with the specified inode.  The portion
 * to be freed is specified by lp->l_start and lp->l_len (already
 * normalized to a "whence" of 0).
 * This is an experimental facility for SVR3.1 and WILL be changed
 * in the next release.  Currently, we only support the special case
 * of l_len == 0, meaning free to end of file.
 *
 * Blocks are freed in reverse order.  This FILO algorithm will tend to
 * maintain a contiguous free list much longer than FIFO.
 * See also s5itrunc() in s5iget.c.
 *
 * Bug:  unused bytes in the last retained block are not cleared.
 * This may result in a "hole" in the file that does not read as zeroes.
 */
s5freesp(ip, lp, flag)
register struct inode *ip;
register struct flock *lp;
int flag;
{
	register int i;
	register struct s5inode *s5ip;
	register struct mount *mp;
	register daddr_t bn;
	register daddr_t lastblock;
	register long bsize;
	long nindir;
	daddr_t lastiblock[NIADDR];
	daddr_t save[NADDR];

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	ASSERT((s5ip->s5i_mode & IFMT) == IFREG);
	ASSERT(lp->l_start >= 0);	/* checked by convoff */

	if (lp->l_len != 0) {
		u.u_error = EINVAL;
		return;
	}
	if (ip->i_size <= lp->l_start)
		return;

	/*
	 * Truncation honors mandatory record locking protocol
	 * exactly as writing does.  See s5chklock() in s5rdwri.c.
	 */

	if (!s5access(ip, IMNDLCK)) {
		lp->l_type = F_WRLCK;
		i = (flag & FNDELAY) ? INOFLCK : SLPFLCK|INOFLCK;
		if ((i = reclock(ip, lp, i, 0, lp->l_start)) != 0
			|| lp->l_type != F_UNLCK) {
			u.u_error = i ? i : EAGAIN;
			return;
		}
	}

	/*
	 * Release the direct-load block map (if any).
	 */

	if (s5ip->s5i_map)
		s5freemap(ip);

	/*
	 * Calculate index into inode's block list of last block
	 * we want to keep.  Lastblock is -1 when the file is
	 * truncated to 0.
	 *
	 * Think of the file as consisting of four separate lists
	 * of data blocks:  one list of up to NDADDR blocks pointed
	 * to directly from the inode, and three lists of up to
	 * nindir, nindir**2 and nindir**3 blocks, respectively
	 * headed by the SINGLE, DOUBLE and TRIPLE indirect block
	 * pointers.  For each of the four lists, we're calculating
	 * the index within the list of the last block we want to
	 * keep.  If the index for list i is negative, it means
	 * that said block is not in list i (but perhaps in list i-1),
	 * hence all blocks in list i are to be discarded; if the
	 * index is beyond the end of list i, it means that the
	 * block is not in list i (but perhaps in list i+1), hence
	 * all blocks in list i are to be kept.
	 */

	mp = ip->i_mntdev;
	bsize = mp->m_bsize;
	nindir = FsNINDIR(bsize);
	lastblock = ((lp->l_start + FsBSIZE(bsize)-1) >> FsBSHIFT(bsize)) - 1;
	lastiblock[SINGLE] = lastblock - NDADDR;
	lastiblock[DOUBLE] = lastiblock[SINGLE] - nindir;
	lastiblock[TRIPLE] = lastiblock[DOUBLE] - nindir*nindir;

	/*
	 * Update file size and block pointers in
	 * disk inode before we start freeing blocks.
	 * Also normalize lastiblock values to -1
	 * for calls to s5indirtrunc below.
	 */

	for (i = NADDR-1; i >= 0; i--)
		save[i] = s5ip->s5i_addr[i];

	for (i = TRIPLE; i >= SINGLE; i--)
		if (lastiblock[i] < 0) {
			s5ip->s5i_addr[IB(i)] = 0;
			lastiblock[i] = -1;
		}

	for (i = NDADDR-1; i > lastblock; i--)
		s5ip->s5i_addr[i] = 0;

	ip->i_size = lp->l_start;
	ip->i_flag |= IUPD|ICHG|ISYN;
	s5iupdat(ip, &time, &time);

	/*
	 * Indirect blocks first.
	 */

	for (i = TRIPLE; i >= SINGLE; i--) {
		if ((bn = save[IB(i)]) != 0) {
			s5indirtrunc(mp, bn, lastiblock[i], i);
			if (lastiblock[i] < 0)
				s5free(mp, bn);
		}
		if (lastiblock[i] >= 0)
			return;
	}

	/*
	 * Direct blocks.
	 */

	for (i = NDADDR-1; i > lastblock; i--) {
		if ((bn = save[i]) != 0)
			s5free(mp, bn);
	}
}

s5indirtrunc(mp, bn, lastbn, level)
register struct mount *mp;
daddr_t bn, lastbn;
int level;
{
	register int i;
	register struct buf *bp, *copy;
	register daddr_t *bap;
	long bsize, factor;
	long nindir;
	daddr_t last;

	/*
	 * Calculate index in current block of last block (pointer) to be kept.
	 * A lastbn of -1 indicates that the entire block is going away, so we
	 * need not calculate the index.
	 */

	bsize = mp->m_bsize;
	nindir = FsNINDIR(bsize);
	factor = 1;
	for (i = SINGLE; i < level; i++)
		factor *= nindir;
	last = lastbn;
	if (lastbn > 0)
		last /= factor;

	/*
	 * Get buffer of block pointers, zero those entries corresponding to
	 * blocks to be freed, and update on-disk copy first.  (If the entire
	 * block is to be discarded, there's no need to zero it out and
	 * rewrite it, since there are no longer any pointers to it, and it
	 * will be freed shortly by the caller anyway.)
	 * Note potential deadlock if we run out of buffers.  One way to
	 * avoid this might be to use statically-allocated memory instead;
	 * you'd have to make sure that only one process at a time got at it.
	 */

	copy = geteblk();
	bp = bread(mp->m_dev, bn, FsBSIZE(bsize));
	if (bp->b_flags & B_ERROR) {
		brelse(copy);
		brelse(bp);
		return;
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, copy->b_un.b_addr, FsBSIZE(bsize));
	if (last < 0)
		brelse(bp);
	else {
		bzero((caddr_t)&bap[last+1],
			(int)(nindir - (last+1)) * sizeof(daddr_t));
		bwrite(bp);
	}
	bap = copy->b_un.b_daddr;

	/*
	 * Recursively free totally unused blocks.
	 */

	for (i = nindir-1; i > last; i--)
		if ((bn = bap[i]) != 0) {
			if (level > SINGLE)
				s5indirtrunc(mp, bn, (daddr_t)-1, level-1);
			s5free(mp, bn);
		}

	/*
	 * Recursively free last partial block.
	 */

	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		if ((bn = bap[i]) != 0)
			s5indirtrunc(mp, bn, last, level-1);
	}

	brelse(copy);
}
