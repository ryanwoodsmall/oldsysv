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
/* @(#)nami.c	6.4 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/inode.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/var.h>

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 *
 * func = function called to get next char of name
 *	&uchar if name is in user space
 *	&schar if name is in system space
 * flag = 0 if name is sought
 *	1 if name is to be created
 *	2 if name is to be deleted
 */
struct inode *
namei(func, flag)
register (*func)();
{
	register struct inode *dp;
	register c;
	register char *cp;
	register struct buf *bp;
	daddr_t bn;
	dev_t d;
	off_t eo, off;

	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */

	sysinfo.namei++;
	c = (*func)();
	if (c == '\0') {
		u.u_error = ENOENT;
		return(NULL);
	}
	if (c == '/') {
		if ((dp = u.u_rdir) == NULL)
			dp = rootdir;
		while(c == '/')
			c = (*func)();
		if(c == '\0' && flag != 0) {
			u.u_error = ENOENT;
			return(NULL);
		}
	} else
		dp = u.u_cdir;
	iget(dp->i_dev, dp->i_number);

cloop:
	/*
	 * Here dp contains pointer
	 * to last component matched.
	 */

	if(u.u_error)
		goto out;
	if(c == '\0')
		return(dp);

	/*
	 * If there is another component,
	 * gather up name into users' dir buffer.
	 */


	cp = &u.u_dent.d_name[0];
	while(c != '/' && c != '\0' && u.u_error == 0) {
		if(cp < &u.u_dent.d_name[DIRSIZ])
			*cp++ = c;
		c = (*func)();
	}
	while(cp < &u.u_dent.d_name[DIRSIZ])
		*cp++ = '\0';
	while(c == '/')
		c = (*func)();
	if ((flag == 1) && (c == '\0') && (u.u_error == 0)) {
		cp = &u.u_dent.d_name[0];
		while (cp < &u.u_dent.d_name[DIRSIZ])
			if (*cp++ & 0200)
				u.u_error = EFAULT;
	}

seloop:
	/*
	 * dp must be a directory and
	 * must have X permission.
	 */
	if ((dp->i_mode&IFMT) != IFDIR || dp->i_nlink==0)
		u.u_error = ENOTDIR;
	else
		access(dp, IEXEC);
	if (u.u_error)
		goto out;

	/*
	 * set up to search a directory
	 */
	u.u_offset = 0;
	u.u_count = dp->i_size;
	u.u_pbsize = 0;
	bp = NULL;
	eo = 0;
	if (dp == u.u_rdir)
	if (u.u_dent.d_name[0] == '.')
	if (u.u_dent.d_name[1] == '.')
	if (u.u_dent.d_name[2] == '\0')
		goto cloop;

eloop:

	/*
	 * If at the end of the directory,
	 * the search failed. Report what
	 * is appropriate as per flag.
	 */
	if (u.u_count == 0) {
		if(bp != NULL)
			brelse(bp);
		if(flag==1 && c=='\0') {
			if(access(dp, IWRITE))
				goto out;
			u.u_pdir = dp;
			if (eo)
				u.u_offset = eo;
			u.u_count = sizeof(struct direct);
			bmap(dp, B_WRITE);
			if (u.u_error)
				goto out;
			return(NULL);
		}
		u.u_error = ENOENT;
		goto out;
	}

	/*
	 * Read the next directory block
	 */
	if(bp != NULL)
		brelse(bp);
	sysinfo.dirblk++;
	bn = bmap(dp, B_READ);
	if (u.u_error)
		goto out;
	if (bn < 0) {
		u.u_error = EIO;
		goto out;
	}
	bp = bread(dp->i_dev, bn, 0);
	if (u.u_error) {
		brelse(bp);
		goto out;
	}

	/*
	 * Search directory block. searchdir() returns
	 * offset of matching entry, or empty entry, or -1.
	 */
	cp = bp->b_un.b_addr + u.u_pboff;
	switch(off = searchdir(cp, u.u_pbsize, u.u_dent.d_name)) {
	default:
		cp += off;
		if((u.u_dent.d_ino = ((struct direct *)cp)->d_ino) != 0)
			break;
		if(eo == 0)
			eo = u.u_offset + off;
	case -1:
		u.u_offset += u.u_pbsize;
		u.u_count -= u.u_pbsize;
		goto eloop;
	}

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */
	u.u_offset += off + sizeof(struct direct);
	if(bp != NULL)
		brelse(bp);
	if(flag==2 && c=='\0') {
		if(access(dp, IWRITE))
			goto out;
		return(dp);
	}
	d = dp->i_dev;
	if(u.u_dent.d_ino == ROOTINO)
	if(dp->i_number == ROOTINO)
	if(u.u_dent.d_name[1] == '.') {
		register struct mount *mp;

		for(mp = &mount[1]; mp < (struct mount *)v.ve_mount; mp++)
			if(mp->m_flags == MINUSE)
			if(mp->m_dev == d) {
				iput(dp);
				dp = mp->m_inodp;
				dp->i_count++;
				plock(dp);
				goto seloop;
			}
	}
	iput(dp);
	dp = iget(d, u.u_dent.d_ino);
	if(dp == NULL)
		return(NULL);
	goto cloop;

out:
	iput(dp);
	return(NULL);
}

