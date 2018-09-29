/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)3b2s52k:fs/s52k/s52ksys3.c	10.4"
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

extern struct buf *s52kgeteblk();
extern struct buf *s52kbread();

extern struct mount s5_2048mnt;

s52kstatf(ip, st)
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
	bp = s52kbread(ip->i_dev, FsITOD(bsize, ip->i_number), FsBSIZE(bsize));
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(bsize, ip->i_number);
	st->st_atime = dp->di_atime;
	st->st_mtime = dp->di_mtime;
	st->st_ctime = dp->di_ctime;
	s52kbrelse(bp);
}

/* s52k fcntl */

s52kfcntl(ip, cmd, arg, flag, offset)
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
			s52kfreesp(ip, &bf, flag);
		break;

	default:
		u.u_error = EINVAL;
	}
}

/* s52k ioctl */
s52kioctl(ip, cmd, arg, flag)
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
s52kmount(bip, mp, flags, dataptr, datalen)
register struct inode *bip;
register struct mount *mp;
int flags;
char *dataptr;
int datalen;
{
	register dev_t dev;
	register struct filsys *fp;
	register struct inode *ip;
	int rdonly;
	extern short s52kfstyp;
	struct inode in;
	struct s5inode s52kin;

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

	mp->m_bufp = (caddr_t)s52kgeteblk();
	fp = getfs(mp);
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	u.u_error = 0;
	ip = &in;	/* need to use a dummy inode since bip may not
			 * be an s52k inode. (bip is the special dev arg.
			 * passed in mount.)
			 */
	mp->m_bsize=2048;
	ip->i_ftype = IFBLK;
	ip->i_rdev = bip->i_rdev;
	ip->i_fstyp = s52kfstyp;	/* index into FSS table */
	ip->i_mntdev = mp;
	ip->i_fsptr = (int *)&s52kin;
	s52kin.s5i_mode = IFBLK;
	s52kreadi(ip);			/* read the superblock */
	if (u.u_error){
		goto out;
	}
	fp->s_fmod = 0;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;
	fp->s_ronly = rdonly;
	if (fp->s_magic != FsMAGIC) {
		u.u_error = EINVAL;
		goto out;
	}
	/* determine fstyp */
	switch ((int) fp->s_type) {
	case Fs2b:
		/*
		 * check root inode and directory this could be
		 * an old style 3b15 2KB file system.
		 */
		if(s52kchklblksz(ip,2048)){
			/* not a 2k must be 1k file system */
			u.u_error = EINVAL;
			goto out;
		}
		/* it is a 2K fall thru */
	case Fs4b:
		/*
		 * Invalidate buffer containing the superblock
		 * read by s52kreadi().
		 */
		s52kbinval(mp->m_dev);
		mp->m_bsize = 2048;
		break;
	default:
		/* S52K code only supports 2K-byte file systems */
		u.u_error = EINVAL;
		goto out;
	}

	/* update state field in superblock for a writable file system */

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
			s52kwritei(ip);
			if (u.u_error) {
				u.u_error = EROFS;
				goto out;
			}
		} else {
			u.u_error = ENOSPC;
			goto out;
		}
	}

	mp->m_fstyp = s52kfstyp;	/* index into fs switch table */

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
	s52kbinval(dev);
	(*bdevsw[bmajor(dev)].d_close)(dev, rdonly, OTYP_MNT);
	s52kbrelse(mp->m_bufp);
}

/*
 * the umount system call.
 */
s52kumount(mp)
register struct mount *mp;
{
	dev_t dev;
	register struct inode *ip, *rip;
	register struct filsys *fp;
	struct inode in;
	struct s5inode s5in;

	dev = mp->m_dev;

	if (dev & Fs2BLK)
		cmn_err(CE_WARN, "s52kumount dev has Fs2BLK set\n");

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
	s52kiupdat(rip, &time, &time);
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
		s52kbflush(dev);
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
		s52kwritei(ip);
		u.u_error = 0;
	}
	(*bdevsw[bmajor(dev)].d_close)(dev, 0, OTYP_MNT);
	if (u.u_error) {
		prele(rip);
		return;
	}
	punmount(mp);
	s52kbinval(dev);
	s52kbrelse(mp->m_bufp);
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

s52kstatfs(ip, sp, ufstyp)
register struct inode *ip;
register struct statfs *sp;
int  ufstyp;
{
	register struct filsys *fp;
	register struct buf *bp;
	struct inode in;
	struct s5inode s5in;
	extern short s52kfstyp;
	dev_t dev;

	if (ufstyp) {
		/*
		 * File system not mounted.  The inode pointer
		 * refers to a device from which the superblock
		 * must be read.
		 */
		dev = ip->i_rdev;
		if (ip->i_ftype != IFBLK || bmajor(dev) < 0
		  || bmajor(dev) >= bdevcnt) {
			/* Character devices not supported for now. */
			u.u_error = EINVAL;
			return;
		}
		(*bdevsw[bmajor(dev)].d_open)(dev, 0, OTYP_LYR);
		if (u.u_error)
			return;
		bp = s52kgeteblk();
		fp = (struct filsys *)bp->b_un.b_addr;
		in.i_ftype = IFBLK;
		in.i_rdev = dev;
		in.i_fstyp = ufstyp;
		in.i_fsptr = (int *)&s5in;
		in.i_mntdev = &s5_2048mnt;
		s5in.s5i_mode = IFBLK;
		u.u_offset = SUPERBOFF;
		u.u_count = sizeof(struct filsys);
		u.u_base = (caddr_t)fp;
		u.u_segflg = 1;
		s52kreadi(&in);
		if (u.u_error)
			goto out;
	} else
		fp = getfs(ip->i_mntdev);
	if (fp->s_magic != FsMAGIC) {
		u.u_error = EINVAL;
		goto out;
	}

	/* determine block size */
	switch ((int) fp->s_type) {
	case Fs2b:
		if(ufstyp)
			/*
			 * check root inode and directory this could be
			 * an old style 3b15 2KB file system. This file
			 * system has a value of 2 for s_type. 
			 */
			if(s52kchklblksz(&in,2048)){
				/* must be 1k file system */
				u.u_error = EINVAL;
				goto out;
			}
		/* it is a 2K fall thru */
	case Fs4b:
		sp->f_bsize = 2048;
		break;
	default:
		/* S52K code only supports 2K-byte file systems */
		u.u_error = EINVAL;
		goto out;
	}
	sp->f_fstyp = s52kfstyp;
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
		s52kbinval(dev);	/* invalidate superblock buffer */
		s52kbrelse(bp);
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
 * See also s52kitrunc() in s52kiget.c.
 *
 * Bug:  unused bytes in the last retained block are not cleared.
 * This may result in a "hole" in the file that does not read as zeroes.
 */
s52kfreesp(ip, lp, flag)
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

	if (!s52kaccess(ip, IMNDLCK)) {
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
		s52kfreemap(ip);

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
	 * for calls to s52kindirtrunc below.
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
	s52kiupdat(ip, &time, &time);

	/*
	 * Indirect blocks first.
	 */

	for (i = TRIPLE; i >= SINGLE; i--) {
		if ((bn = save[IB(i)]) != 0) {
			s52kindirtrunc(mp, bn, lastiblock[i], i);
			if (lastiblock[i] < 0)
				s52kfree(mp, bn);
		}
		if (lastiblock[i] >= 0)
			return;
	}

	/*
	 * Direct blocks.
	 */

	for (i = NDADDR-1; i > lastblock; i--) {
		if ((bn = save[i]) != 0)
			s52kfree(mp, bn);
	}
}

s52kindirtrunc(mp, bn, lastbn, level)
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

	copy = s52kgeteblk();
	bp = s52kbread(mp->m_dev, bn, FsBSIZE(bsize));
	if (bp->b_flags & B_ERROR) {
		s52kbrelse(copy);
		s52kbrelse(bp);
		return;
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, copy->b_un.b_addr, FsBSIZE(bsize));
	if (last < 0)
		s52kbrelse(bp);
	else {
		bzero((caddr_t)&bap[last+1],
			(int)(nindir - (last+1)) * sizeof(daddr_t));
		s52kbwrite(bp);
	}
	bap = copy->b_un.b_daddr;

	/*
	 * Recursively free totally unused blocks.
	 */

	for (i = nindir-1; i > last; i--)
		if ((bn = bap[i]) != 0) {
			if (level > SINGLE)
				s52kindirtrunc(mp, bn, (daddr_t)-1, level-1);
			s52kfree(mp, bn);
		}

	/*
	 * Recursively free last partial block.
	 */

	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		if ((bn = bap[i]) != 0)
			s52kindirtrunc(mp, bn, last, level-1);
	}

	s52kbrelse(copy);
}

/*
 * Using the raw device field of an inode, try to access part of a
 * file system.  The block size to use is testsize.  A successful
 * access:
 *		- reads the first two inodes in the i-list
 *		- verifies inode #2 is for the root directory
 *		- reads in the first data block of the directory
 *		- verifies first two directory entries are valid
 *
 * If any of the above steps fails, the access attempt fails.
 */
s52kchklblksz(ip, testsize)

register struct inode * ip;
int testsize;

{
	struct dinode twoinodes[2];
	struct direct  * dirs;
	daddr_t addr;
	register char * p1;
	register char * p2;
	int count;
	struct buf * bp;

	u.u_offset = testsize * 2;
	u.u_count = sizeof(struct dinode) * 2;
	u.u_base = (caddr_t)&twoinodes[0];
	u.u_segflg = 1;
	s52kreadi(ip);
	if(u.u_error) {
		u.u_error = 0;
		return(-1);
	}
	if((twoinodes[1].di_mode & IFMT) != IFDIR)
		return(-1);
	if(twoinodes[1].di_nlink < 2)
		return(-1);
	if((twoinodes[1].di_size % sizeof(struct direct)) != 0)
		return(-1);

	p1 = (char *)&addr;
	p2 = twoinodes[1].di_addr;
	*p1++ = 0;
	for(count = 0; count < 3; count++)
		*p1++ = *p2++;

	bp = s52kbread(ip->i_rdev,addr,testsize);
	if((bp->b_resid) || (bp->b_flags&B_ERROR)) {
		u.u_error = 0;
		s52kbrelse(bp);
		return(-1);
	}
	dirs = (struct direct *) bp->b_un.b_addr;
	if(dirs[0].d_ino != 2 || dirs[1].d_ino != 2){
		s52kbrelse(bp);
		return(-1);
	}
	if((dirs[0].d_name[0] != '.') || (dirs[0].d_name[1] != '\0')) {
		s52kbrelse(bp);
		return(-1);
	}
	if((dirs[1].d_name[0] != '.') || (dirs[1].d_name[1] != '.' )  ||
		(dirs[1].d_name[2] != '\0'))  {
		s52kbrelse(bp);
		return(-1);
	}
	s52kbrelse(bp);
	return(0);
}
