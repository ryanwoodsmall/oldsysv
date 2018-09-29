/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5sys2.c	10.9"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/dirent.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/mount.h"
#include "sys/fstyp.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

s5getdents(ip, bufpt, bufsz)
register struct inode *ip;
char *bufpt;
int bufsz; /* the size of the user's buffer */
{
	register struct buf *bp, *xbp;
	int bn, i, j;
	int k; /* the total number to be read from disk */
	int m;	/* the total number of bytes returned  in temp block */
	int num = 0;	/* the total number of bytes returned */
	int n;	/* number of bytes left to be read from disk block */
	long ino;
	int dirsz;	/* the displacement of d_name in dirent struct */
	ushort reclen;
	struct dirent *dirent = NULL;
	struct direct *s5dir;

	/* Check if on directory entry boundary */
	if (u.u_offset%sizeof(struct direct)) {  
		u.u_error = ENOENT;
		goto fail;
	}
	u.u_count = ip->i_size;
	u.u_base = bufpt;
	xbp = geteblk();
	if (u.u_error) {
		brelse(xbp);
		goto fail;
	}
	dirsz = dirent->d_name - (char *)dirent;
	while (u.u_count > 0) {
		bn = s5bmap(ip, B_READ);
		if (u.u_error) {
			brelse(xbp);
			goto fail;
		}
		if ((n = u.u_pbsize) == 0)
			break;
		k = n;		/* save number to be read */
		bp = bread(ip->i_dev, bn, FsBSIZE((ip->i_mntdev)->m_bsize));
		if (u.u_error) {
			brelse(bp);
			brelse(xbp);
			goto fail;
		}
		s5dir = (struct direct *)(bp->b_un.b_addr + u.u_pboff);
		dirent = (struct dirent *)xbp->b_un.b_addr;
		m = 0;
		/*
		 * The following loop fills a block with
		 * fs-independent directory entries.
		 */
		while (n > 0) {
			if ((ino = (long)s5dir->d_ino)==0) {
				n-= sizeof(struct direct);
				s5dir++;
				continue;
			}
			for (i = 0; i < DIRSIZ; i++)
				if (s5dir->d_name[i] == '\0')
					break;

			/*
			 * Save the name length.
			 */
			j = i;
			/*
			 * The following rounds the size of a
			 * file system independent directory entry up to
			 * the next fullword boundary, if it is
			 * not already on a fullword boundary.
			 */
			reclen = (dirsz + i + NBPW)&~(NBPW-1);
			if ((m + reclen > SBUFSIZE) || (num + m + reclen > bufsz)) {
				/*
				 * If buffer full or all done, copy
				 * user space.
				 */
				if (copyout((caddr_t)xbp->b_un.b_addr, u.u_base, m)) {
					u.u_error = EFAULT;
					brelse(bp);
					brelse(xbp);
					goto fail;
				}
				num += m;
				if (num + reclen > bufsz) {
					u.u_offset += k - n;
					brelse(bp);
					/*
					 * Check if there is enough 
					 * room to return at least 
					 * one entry.
					 */
					if (num == 0) {
						u.u_error = EINVAL;
						brelse(xbp);
						goto fail;
					}
					goto out;
				}
				u.u_base += m;
				m = 0;
				dirent = (struct dirent *)xbp->b_un.b_addr;
			} else { /* put another entry in buffer */
				m +=reclen;
				dirent->d_reclen = reclen;
				dirent->d_ino = ino;
				dirent->d_off = u.u_offset + k - n + sizeof(struct direct);
				n -= sizeof(struct direct);
				for (i = 0; i < j; i++)
					dirent->d_name[i] = s5dir->d_name[i];
				dirent->d_name[j] = '\0';
				s5dir++;
				dirent = (struct dirent *)(xbp->b_un.b_addr + m);
			}
		}
		u.u_count -= k - n;
		if (copyout((caddr_t)xbp->b_un.b_addr, u.u_base, m)) {
			u.u_error = EFAULT;
			brelse(bp);
			brelse(xbp);
			goto fail;
		}
		u.u_offset += k - n;
		num += m;
		u.u_base += m;
		brelse(bp);
	}
out:
	brelse(xbp);
	return(num);
fail:
	return(-1);
}
