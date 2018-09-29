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
/* @(#)iget.c	6.4 */
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/ino.h"
#include "sys/filsys.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/stat.h"
#include "sys/conf.h"

/*
 *	inode hashing
 */
#define	NHINO	128
#define ihash(X)	(&hinode[(int) (X) & (NHINO-1)])

struct	hinode	{
	struct	inode	*i_forw;
	struct	inode	*i_back;
} hinode[NHINO];

struct	ifreelist {
	int	pad[2];			/* must match struct inode ! */
	struct	inode	*av_forw;
	struct 	inode	*av_back;
} ifreelist;


/*
 * initialize inodes
 */
inoinit()
{
	register struct inode *ip;
	register int i;

	ifreelist.av_forw = ifreelist.av_back = (struct inode *) &ifreelist;
	for(i = 0; i < NHINO; i++)
		hinode[i].i_forw=hinode[i].i_back=(struct inode *) &hinode[i];
	for(i = 0, ip = inode; i < v.v_inode; i++, ip++) {
		ip->i_forw = ip->i_back = ip;
		(ifreelist.av_forw)->av_back = ip;
		ip->av_forw = ifreelist.av_forw;
		ifreelist.av_forw = ip;
		ip->av_back = (struct inode *) &ifreelist;
	}
}

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure), honor the locking protocol.
 * If it is not in core, read it in from the specified device.
 * If the inode is mounted on, perform the indicated indirection.
 * In all cases, a pointer to a locked inode structure is returned.
 *
 * printf warning: inode table overflow -- if the inode structure is full
 * panic: no imt -- if the mounted filesystem is not in the mount table.
 *	"cannot happen"
 */

struct inode *
iget(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct inode *ip;
	register struct hinode *hip;
	struct inode *iread();

	sysinfo.iget++;
loop:
	hip = ihash(ino);
	for(ip = hip->i_forw; ip != (struct inode *) hip; ip = ip->i_forw)
		if(ino == ip->i_number && dev == ip->i_dev)
			goto found;

	if((ip = ifreelist.av_forw) == (struct inode *) &ifreelist) {
		printf("inode table overflow\n");
		syserr.inodeovf++;
		u.u_error = ENFILE;
		return(NULL);
	}

	ip->av_back->av_forw = ip->av_forw;	/* remove from free list */
	ip->av_forw->av_back = ip->av_back;

	ip->i_back->i_forw = ip->i_forw;	/* remove from old hash list */
	ip->i_forw->i_back = ip->i_back;

	hip->i_forw->i_back = ip;	/* insert into new hash list */
	ip->i_forw = hip->i_forw;
	hip->i_forw = ip;
	ip->i_back = (struct inode *) hip;

	/*
	 *	if a block number list was allocated for this file
	 *	(because it is a 413), then free the space now since
	 *	we are going to reuse this table slot for a different
	 *	inode. see the code in mapreg.
	 */
	if (ip->i_map)
		freeblklst(ip);

	ip->i_dev = dev;
	ip->i_number = ino;
	ip->i_flag = ILOCK;
	ip->i_count = 1;
	ip->i_lastr = 0;
	return(iread(ip));
found:
	if((ip->i_flag&ILOCK) != 0) {
		ip->i_flag |= IWANT;
		sleep((caddr_t)ip, PINOD);
		goto loop;
	}
	if((ip->i_flag&IMOUNT) != 0) {
		register struct mount *mp;

		for(mp = &mount[0]; mp < (struct mount *)v.ve_mount; mp++)
		if(mp->m_inodp == ip) {
			dev = mp->m_dev;
			ino = ROOTINO;
			if(ip = mp->m_mount)
				goto found;
			else
				goto loop;
		}
		panic("no imt");
	}
	if(ip->i_count == 0) {
				/* remove from freelist */
		ip->av_back->av_forw = ip->av_forw;
		ip->av_forw->av_back = ip->av_back;
	}
	ip->i_count++;
	ip->i_flag |= ILOCK;
	return(ip);
}

struct inode *
iread(ip)
register struct inode *ip;
{
	register char *p1, *p2;
	register struct dinode *dp;
	struct buf *bp;
	unsigned i;

	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	if (u.u_error) {
		brelse(bp);
		iput(ip);
		return(NULL);
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = 0;
	}
	brelse(bp);
	return(ip);
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(ip)
register struct inode *ip;
{
	if(ip->i_count == 1) {
		ip->i_flag |= ILOCK;
		if(ip->i_nlink <= 0) {
			itrunc(ip);
			ip->i_mode = 0;
			ip->i_flag |= IUPD|ICHG;
			ifree(ip->i_dev, ip->i_number);
		}
		/*
		 *	if a block number list was allocated for this file
		 *	 then free the space now if the file
		 *	has been modified. see mapreg
		 */
		if ((ip->i_flag & IUPD) && ip->i_map)
			freeblklst(ip);

		if(ip->i_flag&(IACC|IUPD|ICHG))
			iupdat(ip, &time, &time);
		prele(ip);
		ip->i_flag = 0;
		ip->i_count = 0;
		ifreelist.av_back->av_forw = ip;
		ip->av_forw = (struct inode *) &ifreelist;
		ip->av_back = ifreelist.av_back;
		ifreelist.av_back = ip;
		return;
	}
	ip->i_count--;
	prele(ip);
}

/*
 * Update the inode with the current time.
 */
iupdat(ip, ta, tm)
register struct inode *ip;
time_t *ta, *tm;
{
	register struct buf *bp;
	struct dinode *dp;
	register char *p1;
	char *p2;
	unsigned i;

	if(getfs(ip->i_dev)->s_ronly) {
		if(ip->i_flag&(IUPD|ICHG))
			u.u_error = EROFS;
		ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
		return;
	}
	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return;
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	p1 = (char *)dp->di_addr;
	p2 = (char *)ip->i_addr;
	if ((ip->i_mode&IFMT)==IFIFO) {
		for (i=0; i<NFADDR; i++) {
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
			if (*p2++ != 0)
				printf("iaddress > 2^24\n");
		}
		for (; i<NADDR; i++) {
			*p1++ = 0;
			*p1++ = 0;
			*p1++ = 0;
		}
	} else
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
		if(*p2++ != 0)
			printf("iaddress > 2^24\n");
	}
	if(ip->i_flag&IACC)
		dp->di_atime = *ta;
	if(ip->i_flag&IUPD)
		dp->di_mtime = *tm;
	if(ip->i_flag&ICHG)
		dp->di_ctime = time;
	if(ip->i_flag&ISYN)
		bwrite(bp);
	else
		bdwrite(bp);
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Free all the disk blocks associated with the specified inode structure.
 * The blocks of the file are removed in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer than FIFO.
 */
itrunc(ip)
register struct inode *ip;
{
	register i;
	dev_t dev;
	daddr_t bn;

	i = ip->i_mode & IFMT;
	if (i!=IFREG && i!=IFDIR)
		return;
	dev = ip->i_dev;
	for(i=NADDR-1; i>=0; i--) {
		bn = ip->i_addr[i];
		if(bn == (daddr_t)0)
			continue;
		ip->i_addr[i] = (daddr_t)0;
		switch(i) {

		default:
			free(dev, bn);
			break;

		case NADDR-3:
			tloop(dev, bn, 0, 0);
			break;

		case NADDR-2:
			tloop(dev, bn, 1, 0);
			break;

		case NADDR-1:
			tloop(dev, bn, 1, 1);
		}
	}
	if (ip->i_map)
		freeblklst(ip);
	ip->i_size = 0;
	ip->i_flag |= IUPD|ICHG;
}

tloop(dev, bn, f1, f2)
dev_t dev;
daddr_t bn;
{
	register i;
	register struct buf *bp;
	register daddr_t *bap;
	daddr_t nb;

	bp = NULL;
	for(i=FsNINDIR(dev)-1; i>=0; i--) {
		if(bp == NULL) {
			bp = bread(dev, bn);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return;
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if(nb == (daddr_t)0)
			continue;
		if(f1) {
			brelse(bp);
			bp = NULL;
			tloop(dev, nb, f2, 0);
		} else
			free(dev, nb);
	}
	if(bp != NULL)
		brelse(bp);
	free(dev, bn);
}

/*
 * Make a new file.
 */
struct inode *
maknode(mode)
register mode;
{
	register struct inode *ip;

	if((mode&IFMT) == 0)
		mode |= IFREG;
	mode &= ~u.u_cmask;
	ip = ialloc(u.u_pdir->i_dev, mode, 1);
	if(ip == NULL) {
		iput(u.u_pdir);
		return(NULL);
	}
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with parameters left as side effects
 * to a call to namei.
 */
wdir(ip)
struct inode *ip;
{

	u.u_dent.d_ino = ip->i_number;
	u.u_count = sizeof(struct direct);
	u.u_segflg = 1;
	u.u_base = (caddr_t)&u.u_dent;
	u.u_fmode = FWRITE;
	writei(u.u_pdir);
	iput(u.u_pdir);
}

/*
 * Search inodes for those on dev
 * Purge any cached inodes.
 * Returns -1 if an active inode is
 * found, otherwise 0
 */
iflush(dev)
register dev_t dev;
{
	register struct inode *ip;
	register i;

	for (i = 0, ip = inode; i < v.v_inode; i++, ip++) {
		if (ip->i_dev == dev) {
			if(ip->i_count == 0) {
					/* remove from hash list */
				ip->i_back->i_forw = ip->i_forw;
				ip->i_forw->i_back = ip->i_back;
				ip->i_forw = ip->i_back = ip;
			} else
				return(-1);
		}
	}
	return(0);
}

/*
 * iinit is called once (from main) very early in initialization.
 * It reads the root's super block and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super block.
 * Usually because of an IO error.
 */
iinit()
{
	register struct buf *cp;
	register struct filsys *fp;
	struct inode iinode;

	(*bdevsw[bmajor(rootdev)].d_open)(minor(rootdev), 1);
	(*bdevsw[bmajor(pipedev)].d_open)(minor(pipedev), 1);
	(*bdevsw[bmajor(swapdev)].d_open)(minor(swapdev), 1);
	cp = geteblk();
	fp = cp->b_un.b_filsys;
	iinode.i_mode = IFBLK;
	iinode.i_rdev = rootdev;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t)fp;
	u.u_segflg = 1;
	readi(&iinode);
	if (u.u_error)
		panic("iinit");
	mount[0].m_bufp = cp;
	mount[0].m_flags = MINUSE;
	mount[0].m_dev = brdev(rootdev);
	if (fp->s_magic != FsMAGIC)
		fp->s_type = Fs1b;	/* assume old file system */
	if (fp->s_type == Fs2b)
		mount[0].m_dev |= Fs2BLK;
#if ns32000
	else
		panic("root must be 1k F.S.");
#endif
	rootdev = mount[0].m_dev;
	if (brdev(pipedev) == brdev(rootdev))
		pipedev = rootdev;
	fp->s_flock = 0;
	fp->s_ilock = 0;
	fp->s_ronly = 0;
	fp->s_ninode = 0;
	fp->s_inode[0] = 0;

	clkset(fp->s_time);
}


/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
register struct inode *ip;
struct stat *ub;
{
	register struct dinode *dp;
	register struct buf *bp;
	struct stat ds;

	if(ip->i_flag&(IACC|IUPD|ICHG))
		iupdat(ip, &time, &time);
	/*
	 * first copy from inode table
	 */
	ds.st_dev = brdev(ip->i_dev);
	ds.st_ino = ip->i_number;
	ds.st_mode = ip->i_mode;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = (dev_t)ip->i_rdev;
	ds.st_size = ip->i_size;
	/*
	 * next the dates in the disk
	 */
	bp = bread(ip->i_dev, FsITOD(ip->i_dev, ip->i_number));
	dp = bp->b_un.b_dino;
	dp += FsITOO(ip->i_dev, ip->i_number);
	ds.st_atime = dp->di_atime;
	ds.st_mtime = dp->di_mtime;
	ds.st_ctime = dp->di_ctime;
	brelse(bp);
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}
