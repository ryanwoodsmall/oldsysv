/* @(#)nami.c	1.1 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/var.h"

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
int (*func)();
{
	register struct inode *dp;
	register c;
	register char *cp;
	struct buf *bp;
	int i;
	dev_t d;
	off_t eo;

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
	eo = 0;
	bp = NULL;
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

	if(u.u_offset >= dp->i_size) {
		if(bp != NULL)
			brelse(bp);
		if(flag==1 && c=='\0') {
			if(access(dp, IWRITE))
				goto out;
			u.u_pdir = dp;
			if (eo)
				u.u_offset = eo - sizeof(struct direct);
			bmap(dp, (daddr_t)(u.u_offset>>BSHIFT), B_WRITE);
			if (u.u_error)
				goto out;
			return(NULL);
		}
		u.u_error = ENOENT;
		goto out;
	}

	/*
	 * If offset is on a block boundary,
	 * read the next directory block.
	 * Release previous if it exists.
	 */

	if((u.u_offset&BMASK) == 0) {
		daddr_t bn;

		if(bp != NULL)
			brelse(bp);
		sysinfo.dirblk++;
		bn = bmap(dp, (daddr_t)(u.u_offset>>BSHIFT), B_READ);
		if (u.u_error)
			goto out;
		if (bn < 0) {
			u.u_error = EIO;
			goto out;
		}
		bp = bread(dp->i_dev, bn);
		if (u.u_error) {
			brelse(bp);
			goto out;
		}
	}

	/*
	 * Note first empty directory slot
	 * in eo for possible creat.
	 * String compare the directory entry
	 * and the current component.
	 * If they do not match, go back to eloop.
	 */

	cp = (char *)&u.u_t[0];
	copyio(paddr(bp)+((unsigned)u.u_offset&BMASK),
		cp, sizeof(struct direct), U_RKD);
	u.u_offset += sizeof(struct direct);
	u.u_dent.d_ino = ((struct direct *)cp)->d_ino;
	if(u.u_dent.d_ino == 0) {
		if(eo == 0)
			eo = u.u_offset;
		goto eloop;
	}
	cp = &((struct direct *)cp)->d_name[0];
	for(i=0; i<DIRSIZ; i++)
		if(*cp++ != u.u_dent.d_name[i])
			goto eloop;

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */

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
	if(u.u_dent.d_name[1] == '.')
		for(i=1; i<v.v_mount; i++)
			if(mount[i].m_flags == MINUSE)
			if(mount[i].m_dev == d) {
				iput(dp);
				dp = mount[i].m_inodp;
				dp->i_count++;
				plock(dp);
				goto seloop;
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

/*
 * Return the next character from the
 * kernel string pointed at by dirp.
 */
schar()
{

	return(*u.u_dirp++ & 0377);
}

/*
 * Return the next character from the
 * user string pointed at by dirp.
 */
uchar()
{
	register c;

	c = fubyte(u.u_dirp++);
	if(c == -1)
		u.u_error = EFAULT;
	return(c);
}
