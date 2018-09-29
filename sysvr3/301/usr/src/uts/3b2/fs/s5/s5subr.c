/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5subr.c	10.11"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/ustat.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5dir.h"
#include "sys/statfs.h"
#include "sys/conf.h"
#include "sys/open.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/fcntl.h"
#include "sys/flock.h"

/*
 * s5bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * Indirect blocks and directory data blocks, when first allocated,
 * are cleared and written synchronously to ensure sanity.
 * Indirect blocks for synchronous writes are also updated synchronously.
 * When convenient, it also leaves the physical
 * block number of the next block of the file in u.u_rablock
 * for use in read-ahead.
 */
daddr_t
s5bmap(ip, readflg)
register struct inode *ip;
{
	register bsize;
	register daddr_t bn;
	register struct s5inode *s5ip;
	register i,j;
	struct mount *mp;
	dev_t dev;
	int raflag;
	struct buf *s5alloc();

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	u.u_rablock = 0;
	raflag = 0;
	{
		register sz, rem, type;

		type = s5ip->s5i_mode&IFMT;
		if (type == IFBLK) {
			dev = ip->i_rdev;
			/* scan mount table to see if this block */
			/* device is mounted. If it is get the block */
			/* size info from the mount table. This is */
			/* is needed to make sure that the Fs macros */
			/* work properly. */
			for (mp = &mount[0];
			    mp < (struct mount *)v.ve_mount; mp++) {
				if ((mp->m_flags&MINUSE) && mp->m_dev == dev)
					break;
			}
			/* If this is true the fs was not mounted */
			if (mp == (struct mount *)v.ve_mount)
				mp = ip->i_mntdev;
		} else {
			mp = ip->i_mntdev;
			dev = ip->i_dev;
		}
		bsize = mp->m_bsize;
		u.u_bsize = bsize;
		u.u_pbdev = dev;
		bn = u.u_offset >> FsBSHIFT(bsize);
		if (bn < 0) {
			u.u_error = EFBIG;
			return((daddr_t)-1);
		}
		if ((s5ip->s5i_lastr + 1) == bn)
			raflag = 1;
		u.u_pboff = u.u_offset & FsBMASK(bsize);
		sz = FsBSIZE(bsize) - u.u_pboff;
		if (u.u_count < sz) {
			sz = u.u_count;
			raflag = 0;
		} else
			s5ip->s5i_lastr = bn;
		u.u_pbsize = sz;
		if (type == IFBLK) {
			if (raflag)
				u.u_rablock = bn + 1;
			return(bn);
		}
		if (readflg == 0) {
			if (type == IFREG
			  && u.u_offset >= (uint)(u.u_limit << SCTRSHFT)) {
				u.u_error = EFBIG;
				return((daddr_t)-1);
			}
		} else {
			if (type == IFIFO) {
				raflag = 0;
				rem = ip->i_size;
			} else
				rem = ip->i_size - u.u_offset;
			if (rem < 0)
				rem = 0;
			if (rem < sz)
				sz = rem;
			if ((u.u_pbsize = sz) == 0)
				return((daddr_t)-1);
		}
	}

	{
		register daddr_t nb;
		register struct buf *bp;
		register sh;
		daddr_t *bap;
		int nshift;

		/*
		 * blocks 0..NADDR-4 are direct blocks
		 */
		if (bn < NADDR-3) {
			i = bn;
			nb = s5ip->s5i_addr[i];
			if (nb == 0) {
				if (readflg || (bp = s5alloc(mp))==NULL)
					return((daddr_t)-1);
				nb = FsPTOL(bsize, bp->b_blkno);
				if ((s5ip->s5i_mode&IFMT) == IFDIR)
					bwrite(bp);
				else
					bdwrite(bp);
				s5ip->s5i_addr[i] = nb;
				ip->i_flag |= IUPD|ICHG;
			}
			if ((i < NADDR-4) && raflag)
				u.u_rablock = s5ip->s5i_addr[i+1];
			return(nb);
		}

		/*
		 * addresses NADDR-3, NADDR-2, and NADDR-1
		 * have single, double, triple indirect blocks.
		 * the first step is to determine
		 * how many levels of indirection.
		 */
		nshift = FsNSHIFT(bsize);
		sh = 0;
		nb = 1;
		bn -= NADDR-3;
		for (j = 3; j > 0; j--) {
			sh += nshift;
			nb <<= nshift;
			if (bn < nb)
				break;
			bn -= nb;
		}
		if (j == 0) {
			u.u_error = EFBIG;
			return((daddr_t)-1);
		}

		/*
		 * fetch the address from the inode
		 */
		nb = s5ip->s5i_addr[NADDR-j];
		if (nb == 0) {
			if (readflg || (bp = s5alloc(mp))==NULL)
				return((daddr_t)-1);
			nb = FsPTOL(bsize, bp->b_blkno);
			bwrite(bp);
			s5ip->s5i_addr[NADDR-j] = nb;
			ip->i_flag |= IUPD|ICHG;
		}

		/*
		 * fetch through the indirect blocks
		 */
		for (; j <= 3; j++) {
			bp = bread(dev, nb, FsBSIZE(bsize));
			if (u.u_error) {
				brelse(bp);
				return((daddr_t)-1);
			}
			bap = bp->b_un.b_daddr;
			sh -= nshift;
			i = (bn>>sh) & FsNMASK(bsize);
			nb = bap[i];
			if (nb == 0) {
				register struct buf *nbp;

				if (readflg || (nbp = s5alloc(mp))==NULL) {
					brelse(bp);
					return((daddr_t)-1);
				}
				nb = FsPTOL(bsize, nbp->b_blkno);
				if (j < 3 || (s5ip->s5i_mode&IFMT) == IFDIR)
					bwrite(nbp);
				else
					bdwrite(nbp);
				bap[i] = nb;
				if (u.u_fmode&FSYNC)
					bwrite(bp);
				else
					bdwrite(bp);
			} else
				brelse(bp);
		}

		/*
		 * calculate read-ahead.
		 */
		if ((i < FsNINDIR(bsize)-1) && raflag)
			u.u_rablock = bap[i+1];
		return(nb);
	}
}

s5update(mp)
register struct mount *mp;
{
	register struct filsys *fp;
	struct inode iinode;
	struct s5inode s5in;
	register struct inode *ip;

	fp = getfs(mp);
	if (fp->s_fmod==0 || fp->s_ilock!=0 ||
	   fp->s_flock!=0 || fp->s_ronly!=0)
		return;
	fp->s_fmod = 0;
	fp->s_time = time;
	ip = &iinode;
	ip->i_ftype = IFBLK;
	ip->i_rdev = mp->m_dev;
	ip->i_fstyp = mp->m_fstyp;
	ip->i_fsptr = (int *)&s5in;
	ip->i_mntdev = mp;
	s5in.s5i_mode = IFBLK;
	u.u_error = 0;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	u.u_fmode = FWRITE|FSYNC;
	s5writei(ip);
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
s5access(ip, mode)
register struct inode *ip;
register mode;
{
	register struct s5inode *s5ip;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	switch (mode) {

	case ISUID:
	case ISGID:
	case ISVTX:
		if ((s5ip->s5i_mode&mode))
			return(0);
		return(1);

	case IMNDLCK:
		if (((s5ip->s5i_mode&IFMT)==IFREG) && 
			(s5ip->s5i_mode & (ISGID|(IEXEC>>3))) == ISGID) 
			return(0);
		return(1);

	case IOBJEXEC:
		if ((s5ip->s5i_mode & IFMT) != IFREG ||
 		   (s5ip->s5i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) 
			goto fail;

	case ICDEXEC:
		mode = IEXEC;
		goto exec;

	case IWRITE:
		if (rdonlyfs(ip->i_mntdev)) {
			u.u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&ITEXT)
			xrele(ip);
		if (ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	case IREAD:
	case IEXEC:
exec:
		if (u.u_uid == 0)
			return(0);
modechk:
		if (u.u_uid != ip->i_uid) {
			mode >>= 3;
			if (u.u_gid != ip->i_gid)
				mode >>= 3;
		}
		if ((s5ip->s5i_mode&mode) != 0)
			return(0);
		goto fail;

	}

fail:
	u.u_error = EACCES;
	return(1);
}

/*
 * s5openi called to allow handler of special files to initialize and
 * validate before actual IO.
 */
s5openi(ip, flag)
register struct inode *ip;
{
	dev_t dev;
	register unsigned int maj;

	dev = (dev_t)ip->i_rdev;
	switch (ip->i_ftype) {

	case IFCHR:
		maj = major(dev);
		if (maj >= cdevcnt)
			goto bad;
		if (u.u_ttyp == NULL)
			u.u_ttyd = dev;
		if (cdevsw[maj].d_str)
			stropen(ip, flag);
		else
			(*cdevsw[maj].d_open)(dev, flag, OTYP_CHR);
		break;

	case IFBLK:
		maj = bmajor(dev);
		if (maj >= bdevcnt)
			goto bad;
		(*bdevsw[maj].d_open)(dev, flag, OTYP_BLK);
		break;

	case IFIFO:
		s5openf(ip, flag);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/* ARGSUSED */
s5closei(ip, flag, count, offset)
register struct inode *ip;
int flag, count;
off_t offset;
{
	register int (*cfunc)();
	int otyp;
	register struct file *fp;
	register int fmt;
	register dev_t dev;

	cleanlocks(ip,USE_PID);
	if (ip->i_sptr && ip->i_ftype != IFDIR)
		strclean(ip);
	if ((unsigned)count > 1)
		return;

	fmt = ip->i_ftype;
	dev = ip->i_rdev;
	/*  **********************************************  */
	/*  Used by 3bnet to pass port number to niclose()  */
	/*  **********************************************  */
	/*  A better solution should be found... */
	if (flag & FNET) {
		switch (fmt) {

		case IFCHR:
			cfunc = cdevsw[major(dev)].d_close;
			otyp = OTYP_CHR;
			break;

		case IFBLK:
			cfunc = bdevsw[bmajor(dev)].d_close;
			otyp = OTYP_BLK;
			break;

		default: /* "Can't happen" */
			cmn_err(CE_WARN, "FNET set on non-device file");
			return;
		}

		(*cfunc) (dev, flag, otyp, (unsigned) offset);
		return;
	}

	switch (fmt) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		otyp = OTYP_CHR;
		break;

	case IFBLK:
		cfunc = bdevsw[bmajor(dev)].d_close;
		otyp = OTYP_BLK;
		break;

	case IFIFO:
		s5closef(ip, flag);

	default:
		return;
	}

	/*
	 * Don't call device close routine if there is any other open 
	 * device inode referring to the same device.
	 */
	if (ip->i_count > 1) 
		goto out;

	for (fp = file; fp < (struct file *)v.ve_file; fp++) {
		register struct inode *tip;

		if (fp->f_count) {
			tip = fp->f_inode;
			if (tip->i_rdev == dev && tip->i_ftype == fmt
			  && tip != ip)
				goto out;
		}
	}
	if (setjmp(u.u_qsav)) {	/* catch half-closes */
		plock(ip);
		goto out;
	}
	if (fmt == IFBLK) {
		register struct mount *mp;

		for (mp = mount; mp < (struct mount *)v.ve_mount; mp++)
			if ((mp->m_flags & MINUSE) && mp->m_dev == dev) {
				(*cfunc)(dev, flag, otyp);
				goto out;
			}
		bflush(dev);
		(*cfunc)(dev, flag, otyp);
		binval(dev);
	} else {
		prele(ip);
		if (cdevsw[major(dev)].d_str)
			strclose(ip, flag);
		else
			(*cfunc)(dev, flag, otyp);
		plock(ip);
	}
out:
	return;
}
