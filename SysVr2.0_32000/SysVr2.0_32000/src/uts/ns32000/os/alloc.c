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
/* @(#)alloc.c	6.3 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/filsys.h"
#include "sys/fblk.h"
#include "sys/page.h"
#include "sys/buf.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/var.h"

typedef	struct fblk *FBLKP;

/*
 * alloc will obtain the next available free disk block from the free list
 * of the specified device.
 * The super block has up to NICFREE remembered free blocks;
 * the last of these is read to obtain NICFREE more . . .
 *
 * no space on dev x/y -- when the free list is exhausted.
 */
struct buf *
alloc(dev)
dev_t dev;
{
	daddr_t bno;
	register struct filsys *fp;
	register struct buf *bp;

	fp = getfs(dev);
	while(fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	do {
		if (fp->s_nfree <= 0)
			goto nospace;
		bno = fp->s_free[--fp->s_nfree];
		if (bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));
	if (fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno);
		if (u.u_error == 0) {
			fp->s_nfree = ((FBLKP)(bp->b_un.b_addr))->df_nfree;
			bcopy((caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
			    (caddr_t)fp->s_free, sizeof(fp->s_free));
		}
		brelse(bp);
		fp->s_flock = 0;
		wakeup((caddr_t)&fp->s_flock);
	}
	if (fp->s_nfree <= 0 || fp->s_nfree > NICFREE) {
		prdev("Bad free count", dev);
		goto nospace;
	}
	bp = getblk(dev, bno);
	clrbuf(bp);
	if (fp->s_tfree) fp->s_tfree--;
	fp->s_fmod = 1;
	return(bp);

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
	delay(5*HZ);
	prdev("no space", dev);
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * place the specified disk block back on the free list of the
 * specified device.
 */
free(dev, bno)
dev_t dev;
daddr_t bno;
{
	register struct filsys *fp;
	register struct buf *bp;

	fp = getfs(dev);
	fp->s_fmod = 1;
	while(fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	if (badblock(fp, bno, dev))
		return;
	if (fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if (fp->s_nfree >= NICFREE) {
		fp->s_flock++;
		bp = getblk(dev, bno);
		((FBLKP)(bp->b_un.b_addr))->df_nfree = fp->s_nfree;
		bcopy((caddr_t)fp->s_free,
			(caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
			sizeof(fp->s_free));
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup((caddr_t)&fp->s_flock);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_tfree++;
	fp->s_fmod = 1;
}

/*
 * Check that a block number is in the range between the I list
 * and the size of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
badblock(fp, bn, dev)
register struct filsys *fp;
daddr_t bn;
dev_t dev;
{

	if (bn < fp->s_isize || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
	return(0);
}

/*
 * Allocate an unused I node on the specified device.
 * Used with file creation.
 * The algorithm keeps up to NICINOD spare I nodes in the
 * super block. When this runs out, a linear search through the
 * I list is instituted to pick up NICINOD more.
 */
struct inode *
ialloc(dev, mode, nlink)
dev_t dev;
{
	register struct filsys *fp;
	register struct inode *ip;
	register i;
	register struct buf *bp;
	struct dinode *dp;
	ino_t ino;
	daddr_t adr;

	fp = getfs(dev);
loop:
	while(fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
	if (fp->s_ninode > 0
	    && (ino = fp->s_inode[--fp->s_ninode])) {
		ip = iget(dev, ino);
		if (ip == NULL)
			return(NULL);
		if (ip->i_mode == 0) {
			/* found inode: update now to avoid races */
			ip->i_mode = mode;
			ip->i_nlink = nlink;
			ip->i_flag |= IACC|IUPD|ICHG|ISYN;
			ip->i_uid = u.u_uid;
			ip->i_gid = u.u_gid;
			ip->i_size = 0;
			for (i=0; i<NADDR; i++)
				ip->i_addr[i] = 0;
			if (fp->s_tinode) fp->s_tinode--;
			fp->s_fmod = 1;
			iupdat(ip, &time, &time);
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iupdat(ip, &time, &time);
		iput(ip);
		goto loop;
	}
	fp->s_ilock++;
	fp->s_ninode = NICINOD;
	ino = FsINOS(dev, fp->s_inode[0]);
	for(adr = FsITOD(dev, ino); adr < fp->s_isize; adr++) {
		bp = bread(dev, adr);
		if (u.u_error) {
			brelse(bp);
			ino += FsINOPB(dev);
			continue;
		}
		dp = bp->b_un.b_dino;
		for(i=0; i<FsINOPB(dev); i++,ino++,dp++) {
			if (fp->s_ninode <= 0)
				break;
			if (dp->di_mode == 0)
				fp->s_inode[--fp->s_ninode] = ino;
		}
		brelse(bp);
		if (fp->s_ninode <= 0)
			break;
	}
	fp->s_ilock = 0;
	wakeup((caddr_t)&fp->s_ilock);
	if (fp->s_ninode > 0) {
		fp->s_inode[fp->s_ninode-1] = 0;
		fp->s_inode[0] = 0;
	}
	if (fp->s_ninode != NICINOD) {
		fp->s_ninode = NICINOD;
		goto loop;
	}
	fp->s_ninode = 0;
	prdev("Out of inodes", dev);
	u.u_error = ENOSPC;
	fp->s_tinode = 0;
	return(NULL);
}

/*
 * Free the specified I node on the specified device.
 * The algorithm stores up to NICINOD I nodes in the super
 * block and throws away any more.
 */
ifree(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct filsys *fp;

	fp = getfs(dev);
	fp->s_tinode++;
	if (fp->s_ilock)
		return;
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD) {
		if (ino < fp->s_inode[0])
			fp->s_inode[0] = ino;
		return;
	}
	fp->s_inode[fp->s_ninode++] = ino;
}

/*
 * getfs maps a device number into a pointer to the incore super block.
 * The algorithm is a linear search through the mount table.
 * A consistency check of the in core free-block and i-node counts.
 *
 * bad count on dev x/y -- the count
 *	check failed. At this point, all
 *	the counts are zeroed which will
 *	almost certainly lead to "no space"
 *	diagnostic
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 */
struct filsys *
getfs(dev)
dev_t dev;
{
	register struct mount *mp;
	register struct filsys *fp;

	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
	if (mp->m_flags == MINUSE && mp->m_dev == dev) {
		fp = mp->m_bufp->b_un.b_filsys;
		if (fp->s_nfree > NICFREE || fp->s_ninode > NICINOD) {
			prdev("bad count", dev);
			fp->s_nfree = 0;
			fp->s_ninode = 0;
		}
		return(fp);
	}
	panic("no fs");
	return(NULL);
}

/*
 * update is the internal name of 'sync'. It goes through the disk
 * queues to initiate sandbagged IO; goes through the I nodes to write
 * modified nodes; and it goes through the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	struct filsys *fp;
	static struct inode uinode;

	if (uinode.i_flag)
		return;
	uinode.i_flag++;
	uinode.i_mode = IFBLK;
	for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if (mp->m_flags == MINUSE) {
			fp = mp->m_bufp->b_un.b_filsys;
			if (fp->s_fmod==0 || fp->s_ilock!=0 ||
			   fp->s_flock!=0 || fp->s_ronly!=0)
				continue;
			fp->s_fmod = 0;
			fp->s_time = time;
			uinode.i_rdev = mp->m_dev;
			u.u_error = 0;
			u.u_offset = SUPERBOFF;
			u.u_count = sizeof(struct filsys);
			u.u_base = (caddr_t)fp;
			u.u_segflg = 1;
			u.u_fmode = FWRITE|FSYNC;
			writei(&uinode);
		}
	for(ip = &inode[0]; ip < (struct inode *)v.ve_inode; ip++)
		if ((ip->i_flag&ILOCK)==0 && ip->i_count!=0
		&& (ip->i_flag&(IACC|IUPD|ICHG))) {
			ip->i_flag |= ILOCK;
			ip->i_count++;
			iupdat(ip, &time, &time);
			iput(ip);
		}
	bflush(NODEV);
	uinode.i_flag = 0;
}
