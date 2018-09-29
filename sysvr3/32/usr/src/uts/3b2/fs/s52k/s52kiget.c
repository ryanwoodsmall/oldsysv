/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)3b2s52k:fs/s52k/s52kiget.c	10.2"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fs/s5macros.h"
#include "sys/fstyp.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fs/s5filsys.h"
#include "sys/stat.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

extern struct s5inode *s5ifreelist;	/* Head of System V inode free list */
extern struct buf *s52kbread();

struct inode *
s52kiread(ip)
register struct inode *ip;
{
	register char *p1, *p2;
	register struct s5inode *s5ip;
	register struct dinode *dp;
	struct buf *bp;
	register unsigned i;

	i = ip->i_mntdev->m_bsize;
	bp = s52kbread(ip->i_dev, FsITOD(i, ip->i_number), FsBSIZE(i));
	if (u.u_error) {
		if (ip->i_fsptr != NULL) {
			/* put s5 inode table entry on free list */
			s5ip = (struct s5inode *)ip->i_fsptr;
			s5ip->s5i_flags = S5IFREE;
			s5ip->s5i_next = s5ifreelist;	
			s5ifreelist = s5ip;
			ip->i_fsptr = NULL;	/* clear fs ptr */
		}
		/* may need pipe semaphore initialization here	*/
		s52kbrelse(bp);
		return(NULL);
	}
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(i, ip->i_number);
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	if (ip->i_fsptr == NULL) {
		/* Get next entry from free list.
		 * Lock table before doing so.
		 */
		if ((s5ip = s5ifreelist) == NULL) {
			printf("System V inode table overflow\n");
			u.u_error = ENFILE;
			s52kbrelse(bp);
			return(NULL);
		}
		if (s5ip->s5i_flags != S5IFREE)
			cmn_err(CE_PANIC,
				"Allocated Entry in s5inode free list\n");
		s5ifreelist = s5ip->s5i_next;
		ip->i_fsptr = (int *)s5ip;
	} else
		s5ip = (struct s5inode *)ip->i_fsptr;

	ASSERT(s5ip != NULL);
	s5ip->s5i_mode = dp->di_mode;
	ip->i_ftype = dp->di_mode&IFMT;
	s5ip->s5i_lastr = 0;
	p1 = (char *)s5ip->s5i_addr;
	s5ip->s5i_gen = dp->di_gen;
	p2 = (char *)dp->di_addr;
	for (i = 0; i < NADDR; i++) {
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
	/* set rdev.  For portability the independent portion of the
	 * inode needs to have its own copy. While we are in the switch,
	 * initialize the FIFO semaphores.
	 */
	switch (s5ip->s5i_mode & IFMT) {
	case IFCHR:
	case IFBLK:
		ip->i_rdev = s5ip->s5i_rdev;
		break;

	case IFIFO:
		s5ip->s5i_frptr = s5ip->s5i_fwptr = 0;
		s5ip->s5i_waite = s5ip->s5i_waitf = 0;

	default:
		ip->i_rdev = 0;
		break;
	}
	s52kbrelse(bp);
	return(ip);
}

s52kiput(ip)
register struct inode *ip;
{
	register struct s5inode *s5ip;

	ASSERT(ip->i_fsptr != NULL);

	if ((s5ip = (struct s5inode *)ip->i_fsptr) == NULL) 
		cmn_err(CE_PANIC, "NULL fs pointer in s52kiput\n");
	if (ip->i_count == 0) {
		/* put s5 inode table entry on free list */
		s5ip->s5i_flags = S5IFREE;
		s5ip->s5i_next = s5ifreelist;	
		s5ifreelist = s5ip;
		ip->i_fsptr = NULL;	/* clear fs ptr */
		return;
	}
	ASSERT(ip->i_count == 1);

	if (ip->i_nlink <= 0) {
		s5ip->s5i_gen++;
		s52kitrunc(ip);
		ip->i_flag |= IUPD|ICHG;
		s52kifree(ip);
	}

	/* If a block number list was allocated for this file
	 * (because it is a 413), then free the space now 
	 * if the file has been modified. 
	 * See the code in region.c/mapreg.
	 */

	if ((ip->i_flag & IUPD) 
	  && ip->i_ftype == IFREG && s5ip->s5i_map)
		s52kfreemap(ip);

	if (ip->i_flag & (IACC|IUPD|ICHG))

		/* Only call iupdat if an ifree has not been done; this
		 * avoids a race whereby an ifree could put an inode on
		 * the freelist, the inode could be allocated, and then
		 * the iupdat could put outdated information into the 
		 * disk inode.
		 */
		s52kiupdat(ip, &time, &time);
}

/*
 * Update the inode with the current time.
 */
s52kiupdat(ip, ta, tm)
register struct inode *ip;
time_t *ta, *tm;
{
	struct buf *bp;
	register struct s5inode *s5ip;
	register struct dinode *dp;
	register char *p1;
	register char *p2;
	register unsigned i;

	if (rdonlyfs(ip->i_mntdev))
		return;
	i = ip->i_mntdev->m_bsize;
	bp = s52kbread(ip->i_dev, FsITOD(i, ip->i_number), FsBSIZE(i));
	if (bp->b_flags & B_ERROR) {
		s52kbrelse(bp);
		return;
	}
	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += FsITOO(i, ip->i_number);
	dp->di_mode = s5ip->s5i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_gen = s5ip->s5i_gen;
	p1 = (char *)dp->di_addr;
	p2 = (char *)s5ip->s5i_addr;
	if (ip->i_ftype == IFIFO) {
		for (i = 0; i < NFADDR; i++) {
			if (*p2++ != 0)
				cmn_err(CE_PANIC,
					"iupdat - fifo iaddress > 2^24");
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
		for (; i < NADDR; i++) {
			*p1++ = 0;
			*p1++ = 0;
			*p1++ = 0;
		}
	} else {
		for (i = 0; i < NADDR; i++) {
			if (*p2++ != 0 && ip->i_ftype != IFCHR
			  && ip->i_ftype != IFBLK)
				cmn_err(CE_PANIC, "iupdat - iaddress > 2^24");
			*p1++ = *p2++;
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
	}
	if (ip->i_flag & IACC)
		dp->di_atime = *ta;
	if (ip->i_flag & IUPD) 
		dp->di_mtime = *tm;
	if (ip->i_flag & ICHG) 
		dp->di_ctime = time;
	if (ip->i_flag & ISYN)
		s52kbwrite(bp);
	else
		s52kbdwrite(bp);
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Free all the disk blocks associated with the specified inode structure.
 * The blocks of the file are removed in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer than FIFO.
 * Update inode first with zero size and block addrs to ensure sanity.
 * Save blocks addrs locally to free.
 */
s52kitrunc(ip)
register struct inode *ip;
{
	register i;
	register struct s5inode *s5ip;
	register struct mount *mp;
	register daddr_t bn;
	daddr_t save[NADDR];

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	i = s5ip->s5i_mode & IFMT;
	if (i != IFREG && i != IFDIR)
		return;

	if (i == IFREG && s5ip->s5i_map)
		s52kfreemap(ip);

	mp = ip->i_mntdev;
	ip->i_size = 0;
	for (i = NADDR - 1; i >= 0; i--) {
		save[i] = s5ip->s5i_addr[i];
		s5ip->s5i_addr[i] = (daddr_t)0;
	}
	ip->i_flag |= IUPD|ICHG|ISYN;
	s52kiupdat(ip, &time, &time);

	for (i = NADDR - 1; i >= 0; i--) {
		bn = save[i];
		if (bn == (daddr_t)0)
			continue;

		switch (i) {

		default:
			s52kfree(mp, bn);
			break;

		case NADDR-3:
			s52ktloop(mp, bn, 0, 0);
			break;

		case NADDR-2:
			s52ktloop(mp, bn, 1, 0);
			break;

		case NADDR-1:
			s52ktloop(mp, bn, 1, 1);
		}
	}
}

s52ktloop(mp, bn, f1, f2)
register struct mount *mp;
daddr_t bn;
{
	dev_t dev;
	register i;
	register struct buf *bp;
	register daddr_t *bap;
	register daddr_t nb;

	dev = mp->m_dev;
	bp = NULL;
	for (i = FsNINDIR(mp->m_bsize) - 1; i >= 0; i--) {
		if (bp == NULL) {
			bp = s52kbread(dev, bn, FsBSIZE(mp->m_bsize));
			if (bp->b_flags & B_ERROR) {
				s52kbrelse(bp);
				return;
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if (nb == (daddr_t)0)
			continue;
		/* move following 2 lines out of "if" so that buffer
		 * guaranteed to be released before calling mfree, thus
		 * avoiding the rare deadlock whereby we would have a
		 * buffer locked here but couldn't get the super block lock,
		 * and someone in alloc would have the super block lock and
		 * would not be able to get the buffer lock that is locked
		 * here.
		 */
		s52kbrelse(bp);
		bp = NULL;
		if (f1)
			s52ktloop(mp, nb, f2, 0);
		else
			s52kfree(mp, nb);
	}
	if (bp != NULL)
		s52kbrelse(bp);
	s52kfree(mp, bn);
}

struct mount s5_2048mnt;
short s52kfstyp;

s52kinit()
{
	register unsigned i;
	extern s52kbinit();

	s52kbinit();		/* initialize 2KB buffer pool */

	/* To save some space we use the s5inode table */

	s5_2048mnt.m_bsize = 2048;
	for (s52kfstyp= 0, i = 0; i < nfstyp; i++) {
		if (fstypsw[i].fs_init == s52kinit) {
			s52kfstyp = i;		/* switch table entry for this fs */ 
			break;
		}
	}
	if (s52kfstyp == 0 || s52kfstyp == nfstyp)
		cmn_err(CE_PANIC, "s52kinit: s52kinit not found in fstypsw\n");

}
