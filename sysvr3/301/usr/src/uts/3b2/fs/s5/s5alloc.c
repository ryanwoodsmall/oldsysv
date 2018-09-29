/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5alloc.c	10.7"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5fblk.h"
#include "sys/buf.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/conf.h"

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
s5alloc(mp)
register struct mount	*mp;
{
	register dev_t dev;
	register daddr_t bno;
	register struct filsys *fp;
	register struct buf *bp;

	dev = mp->m_dev;
	fp = getfs(mp);
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	do {
		if (fp->s_nfree <= 0)
			goto nospace;
		if ((bno = fp->s_free[--fp->s_nfree]) == 0)
			goto nospace;
	} while (s5badblock(fp, bno, dev));
	if (fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno, FsBSIZE(mp->m_bsize));
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
	bp = getblk(dev, bno, FsBSIZE(mp->m_bsize));
	clrbuf(bp);
	if (fp->s_tfree)
		fp->s_tfree--;
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
s5free(mp, bno)
register struct mount	*mp;
register daddr_t bno;
{
	register dev_t dev;
	register struct filsys *fp;
	register struct buf *bp;

	dev = mp->m_dev;
	fp = getfs(mp);
	fp->s_fmod = 1;
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	if (s5badblock(fp, bno, dev))
		return;
	if (fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if (fp->s_nfree >= NICFREE) {
		fp->s_flock++;
		bp = getblk(dev, bno, FsBSIZE(mp->m_bsize));
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
s5badblock(fp, bn, dev)
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
 * Allocate an unused I node on the specified device.  Used with
 * file creation.  The algorithm keeps up to NICINOD spare I nodes
 * in the super block. When this runs out, a linear search through the
 * I list is instituted to pick up NICINOD more.
 */
struct inode *
s5ialloc(mp, mode, nlink, rdev)
struct mount	*mp;
ushort mode;
dev_t rdev;
{
	dev_t	dev;
	register struct filsys *fp;
	register struct inode *ip;
	register struct s5inode *s5ip;
	register i;
	struct buf *bp;
	register struct dinode *dp;
	register ushort ino;
	daddr_t adr;

	dev = mp->m_dev;
	fp = getfs(mp);
loop:
	while (fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
	fp->s_ilock = 1;
loop1:
	if (fp->s_ninode > 0 && (ino = fp->s_inode[--fp->s_ninode])) {
		if ((ip = iget(mp, ino)) == NULL){
			fp->s_ilock = 0;
			wakeup(&fp->s_ilock);
			return(NULL);
		}
		s5ip = (struct s5inode *)ip->i_fsptr;
		if (s5ip->s5i_mode == 0) {
			/* found inode: update now to avoid races */
			s5ip->s5i_mode = mode;
			ip->i_ftype = mode&IFMT;
			ip->i_nlink = nlink;
			ip->i_flag |= IACC|IUPD|ICHG|ISYN;
			ip->i_uid = u.u_uid;
			ip->i_gid = u.u_gid;
			ip->i_size = 0;
			for (i = 0; i < NADDR; i++)
				s5ip->s5i_addr[i] = 0;
			/*
			 * Must check for rdev after address fields
			 * are zeroed because rdev is defined to be
			 * the first address field (s5inode.h).
			 */
			switch (s5ip->s5i_mode & IFMT) {
			case IFCHR:
			case IFBLK:
				if (rdev == -1)
					cmn_err(CE_WARN, "-1 rdev in ialloc\n");
				ip->i_rdev = rdev;
				s5ip->s5i_rdev = rdev;
			}
			if (fp->s_tinode)
				fp->s_tinode--;
			fp->s_fmod = 1;
			s5iupdat(ip, &time, &time);
			fp->s_ilock = 0;
			wakeup(&fp->s_ilock);
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		cmn_err(CE_NOTE, "s5ialloc: inode was already allocated\n");
		s5iupdat(ip, &time, &time);
		fp->s_ilock = 0;
		wakeup(&fp->s_ilock);
		iput(ip);
		goto loop;
	}
	/* only try to rebuild freelist if there are free inodes */
	if (fp->s_tinode > 0) {
		fp->s_ninode = NICINOD;
		ino = FsINOS(mp->m_bsize, fp->s_inode[0]);
		for (adr = FsITOD(mp->m_bsize, ino); adr < fp->s_isize; adr++) {
			bp = bread(dev, adr, FsBSIZE(mp->m_bsize));
			if (u.u_error) {
				brelse(bp);
				ino += FsINOPB(mp->m_bsize);
				continue;
			}
			dp = (struct dinode *)bp->b_un.b_addr;
			for (i = 0; i < FsINOPB(mp->m_bsize); i++,ino++,dp++) {
				if (fp->s_ninode <= 0)
					break;
				if (dp->di_mode == 0)
					fp->s_inode[--fp->s_ninode] = ino;
			}
			brelse(bp);
			if (fp->s_ninode <= 0)
				break;
		}
		if (fp->s_ninode > 0) {
			fp->s_inode[fp->s_ninode-1] = 0;
			fp->s_inode[0] = 0;
		}
		if (fp->s_ninode != NICINOD) {
			fp->s_ninode = NICINOD;
			goto loop1;
		}
	}

	fp->s_ninode = 0;
	fp->s_tinode = 0;
	fp->s_ilock = 0;
	wakeup((caddr_t)&fp->s_ilock);
	prdev("Out of inodes", dev);
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * Free the specified I node on the specified device.
 * The algorithm stores up to NICINOD I nodes in the super
 * block and throws away any more.
 */
s5ifree(ip)
register struct inode	*ip;
{
	register struct filsys *fp;
	register ushort ino;
	register struct s5inode *s5ip;

	/* don't put an already free inode on the free list */
	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	if (s5ip->s5i_mode == 0)
		return;

	ino = ip->i_number;
	fp = getfs(ip->i_mntdev);
	while (fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
	fp->s_ilock = 1;
	s5ip->s5i_mode = 0;		/* zero means inode not allocated */
	/*
	 * Update disk inode from incore slot before putting it on
	 * the freelist; this eliminates a race in the simplex code
	 * which did an ifree() and then an iupdat() in iput().
	 */
	s5iupdat(ip, &time, &time);
	fp->s_tinode++;
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD) {
		if (ino < fp->s_inode[0])
			fp->s_inode[0] = ino;
	} else
		fp->s_inode[fp->s_ninode++] = ino;
	fp->s_ilock--;
	wakeup((caddr_t)&fp->s_ilock);
}
