/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5nami.c	10.19"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5macros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/file.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/conf.h"

#define DOT 1
#define DOTDOT 2
s5namei(p, flagp)
struct nx *p;
register struct argnamei *flagp;
{	
	register char *cp;
	register char *comp;
	register struct inode *dp;
	register struct inode *dip;
	register int i;
	register char *ccp;
	struct buf *bp;
	struct mount *mp;
	struct direct dir;
	char *dirend;
	int found = 0;
	int bn;
	off_t eo;
	struct inode *s5ialloc();
	struct direct x[2];
	off_t saveoff;
	int dotflag = 0;
	int dot = 0;
	int dotdot = 0;
	int emptflags;

	comp = p->comp;
	/*
	 * dp must be a either a directory or a symbolic link
	 */
	dp = p->dp;
#ifdef IFLNK
	if (dp->i_ftype & IFLNK)
		goto symlink;
#endif
	if (*comp == '\0') {
		if (flagp) {
			/*
			 * This branch is executed when a rmdir (NI_RMDIR)
			 * or unlink (NI_DEL) is executed on a remotely
			 * mounted directory.
			 */
			u.u_error = EBUSY;
			goto fail;
		}
		dir.d_ino = dp->i_number;
		goto pass;
	}

	dir.d_ino = 0;
	cp = &dir.d_name[0];
	dirend = &dir.d_name[DIRSIZ];
	while (*comp != '\0' && cp < dirend) 
		*cp++ = *comp++;

	while (cp < dirend)
		*cp++ = '\0';

	u.u_segflg = 1;

	u.u_offset = 0;
	u.u_count = dp->i_size;

	eo = -1;	/* eo is used to store the first empty entry index */
	bp = NULL;

	while (u.u_count) {
		/*
		 * Read the next directory block
		 */
		if (bp != NULL)
			brelse(bp);
		sysinfo.dirblk++;
		bn = s5bmap(dp, B_READ);
		if (u.u_error)
			goto fail;
		if (bn < 0) {
			u.u_error = EIO;
			goto fail;
		}
		bp = bread(dp->i_dev, bn, FsBSIZE((dp->i_mntdev)->m_bsize));
		if (u.u_error) {
			brelse(bp);
			goto fail;
		}
	
		/*
		 * Search directory block. searchdir() returns
		 * offset of matching entry, or empty entry, or -1.
		 */
		cp = bp->b_un.b_addr;
		switch (i = searchdir(cp, u.u_pbsize, dir.d_name)) {
		default:
			cp += i;
			if ((dir.d_ino = ((struct direct *)cp)->d_ino) != 0) {
				/* update offset for NI_DEL */
				u.u_offset += i;
				found++;
				goto skip;
			}
			/* keep track of empty slot */
			if (eo < 0)
				eo = u.u_offset + i;
		case -1:
			u.u_offset += u.u_pbsize;
			u.u_count -= u.u_pbsize;
			continue;
		}
	}
skip:
	if (bp != NULL)
		brelse(bp);
	if (flagp == 0) {
		if (found)
			goto pass;
		u.u_error = ENOENT;
		goto fail;
	}

	u.u_count = sizeof(struct direct);
	u.u_base = (caddr_t)&dir;
	u.u_fmode = FWRITE;
	/* Leave offset alone if the operation is NI_DEL or NI_RMDIR */
	if (eo >= 0 && flagp->cmd != NI_DEL && flagp->cmd != NI_RMDIR)
		u.u_offset = eo;

	switch (flagp->cmd) {
	case NI_LINK:	/* make a link */
		if (found) {
			u.u_error = EEXIST;
			goto fail;
		}
		if (s5access(dp, IWRITE))
			goto fail;
		if (dp->i_dev != flagp->idev) {
			u.u_error = EXDEV;
			goto fail;
		}
		if (flagp->ino == dp->i_number)
			dip = dp;
		else if ((dip = iget(dp->i_mntdev, flagp->ino)) == NULL) {
			u.u_error = ENOENT;
			goto fail;
		}
		if (dip->i_nlink >= MAXLINK) {
			iput(dip);
			u.u_error = EMLINK;
			goto fail;
		}
		dip->i_nlink++;
		dip->i_flag |= ICHG|ISYN;
		s5iupdat(dip, &time, &time);
		dir.d_ino = flagp->ino;
		s5writei(dp);
		if (u.u_error) {
			dip->i_nlink--;
			dip->i_flag |= ICHG;
		}
		if (dip != dp)
			iput(dip);
		/* go to null so that inode will */
		/* be released properly. link does not */
		/* expect an inode pointer to be  */
		/* returned */
		iput(dp);
		goto null;
	case NI_MKNOD:	/* mknod - like exclusive create except for */
			/* mode setting - see below */
	case NI_XCREAT: /* Exclusive create on a file */
		if (found) {
			u.u_error = EEXIST;
			goto fail;
		}
	case NI_CREAT:	/* create a new file */
		if (found) {
			/* Trying to creat an existing directory - */
			/* error */
			if (dp->i_number == dir.d_ino) {
				u.u_error = EISDIR;
				goto fail;
			}
			mp = dp->i_mntdev;
			iput(dp);
			if ((dp = iget(mp, dir.d_ino)) == NULL)
				goto noiputfail;
			flagp->rcode = FSN_FOUND;
			goto done;
		}
		if (s5access(dp, IWRITE))
			goto fail;
		/* Do not permit the setting of an */
		/* unspecified ftype or mode */
		if ((flagp->ftype & (ushort)(~IFMT))) {
			u.u_error = EINVAL;
			goto fail;
		}
		if (flagp->ftype == 0)
			flagp->ftype = IFREG;
		flagp->mode &= MODEMSK;
		flagp->mode &= ~u.u_cmask;
		if (flagp->cmd != NI_MKNOD)
			flagp->mode &= ~ISVTX;
		flagp->mode |= flagp->ftype;
		dip = s5ialloc(dp->i_mntdev, flagp->mode, 1, flagp->idev);
		if (dip == NULL)
			goto fail;
		dir.d_ino = dip->i_number;
		s5writei(dp);
		iput(dp);
		dp = dip;
		if (u.u_error)
			goto fail;
		flagp->rcode = FSN_NOTFOUND;
		goto done;
	case NI_MKDIR:
		/* make a directory */

		if (found) {
			u.u_error = EEXIST;
			goto fail;
		}
		if (dp->i_nlink >= MAXLINK) {
			u.u_error = EMLINK;
			goto fail;
		}
		if (s5access(dp, IWRITE))
			goto fail;
		flagp->mode &= PERMMSK;
		flagp->mode &= ~u.u_cmask;
		flagp->mode |= IFDIR;
		dip = s5ialloc(dp->i_mntdev, flagp->mode, 2, flagp->idev);
		if (dip == NULL)
			goto fail;
		x[0].d_ino = dip->i_number;  /* inumber of new directory */
		x[1].d_ino = dp->i_number;  /* inumber of .. */
		cp = &x[0].d_name[0];
		ccp = &x[1].d_name[0];
		dirend = &x[0].d_name[DIRSIZ];
		while (cp < dirend) {
			*cp++ = '\0';
			*ccp++ = '\0';
		}
		x[0].d_name[0] = x[1].d_name[0] = x[1].d_name[1] = '.';
		u.u_count = 2 * sizeof(struct direct);
		u.u_base = (caddr_t)x;
		saveoff = u.u_offset;
		u.u_offset = 0;
		u.u_segflg = 1;
		s5writei(dip);
		if (u.u_error) {
			dip->i_nlink = 0;
			iput(dip);
			goto fail;
		}
		dir.d_ino = dip->i_number;
		dip->i_flag |= ISYN;
		s5iupdat(dip, &time, &time);
		iput(dip);
		u.u_offset = saveoff;
		u.u_base = (caddr_t)&dir;
		u.u_count = sizeof(struct direct);
		s5writei(dp);
		dp->i_nlink++;	/* update link count for .. */
		dp->i_flag |= ICHG;
		iput(dp);
		goto null;


	case NI_RMDIR:
		if (found == 0) {
			u.u_error = ENOENT;
			goto fail;
		}
		if (s5access(dp, IWRITE)) {
			goto fail;
		}
		if (dp->i_number == dir.d_ino) { /* can't remove . */
			u.u_error = EINVAL;
			goto fail;
		}
		if ((dip = iget(dp->i_mntdev, dir.d_ino)) == NULL)
			goto fail;
		/*
		 * The following checks if we are crossing
		 * a mount point. There is an implicit assumption 
		 * that when crossing a mount point the device
		 * number changes.
		 */
		if (dip->i_dev != dp->i_dev) {
			iput(dip);
			u.u_error = EBUSY;
			goto fail;
		}
		if (dip->i_ftype  != IFDIR) {
			u.u_error = ENOTDIR;
			iput(dip);
			goto fail;
		}
		if (dip == u.u_cdir) {
			u.u_error = EINVAL;
			iput(dip);
			goto fail;
		}
		/* the following checks if the directory is empty */
		u.u_count = dip->i_size;
		saveoff = u.u_offset;
		u.u_offset = 0;
		while (u.u_count) {
			bn = s5bmap(dip, B_READ);
			if (u.u_error) {
				iput(dip);
				goto fail;
			}
			bp = bread(dip->i_dev,bn, FsBSIZE((dip->i_mntdev)->m_bsize));
			if (u.u_error) {
				brelse(bp);
				iput(dip);
				goto fail;
			}
			cp = bp->b_un.b_addr;
			/*
			 * Emptblk returns 1 if the directory is not empty.
			 * If it contains nothing different from . and ..
			 * then the return value indicates whether .
			 * and .. are actually present.
			 */
			if ((emptflags = emptblk(cp, u.u_pbsize)) == -1) {
				u.u_error = EEXIST;
				brelse(bp);
				iput(dip);
				goto fail;
			}
			else {
				dotflag |= emptflags;
				brelse(bp);
				u.u_offset += u.u_pbsize;
				u.u_count -= u.u_pbsize;
			}
		}
		if (dotflag & DOT)
			dot++;
		if (dotflag & DOTDOT)
			dotdot++;
		if (dotdot)
			dp->i_nlink--;
		if (dot)
			dip->i_nlink -= 2;
		else
			dip->i_nlink--;
		dip->i_flag |= ICHG;
		dir.d_ino = 0;
		u.u_count = sizeof(struct direct);
		u.u_base = (caddr_t)&dir;
		u.u_fmode = FWRITE | FSYNC;
		u.u_offset = saveoff;
		s5writei(dp);
		iput(dp);
		iput(dip);
		goto null;
	case NI_DEL:
		/* delete the entry */
		if (found == 0) {
			u.u_error = ENOENT;
			goto fail;
		}
		if (s5access(dp, IWRITE))
			goto fail;
		if (dp->i_number == dir.d_ino) 	/* for '.' */
			dip = dp;
		else
			dip = iget(dp->i_mntdev, dir.d_ino);
		if (dip == NULL)
			goto fail;
		if (dip->i_dev != dp->i_dev) {	/* mounted FS? */
			u.u_error = EBUSY;
			goto delfail;
		}
		if (dip->i_ftype == IFDIR && !suser()) 
			goto delfail;
		if (dip->i_flag&ITEXT)
			xrele(dip);	/* try to free busy text */
		if (dip->i_flag&ITEXT && dip->i_nlink == 1) {
			u.u_error = ETXTBSY;
			goto delfail;
		}
		u.u_fmode = FWRITE|FSYNC;
		dir.d_ino = 0;
		s5writei(dp);
		if (u.u_error) 
			goto delfail;
		dip->i_nlink--;
		dip->i_flag |= ICHG;
delfail:
		/* unlink does not expect any inode to be */
		/* returned so clean up. */
		/* if unlinking '.', avoid doing iput twice. */
		if (dp != dip)
			iput(dip);
		if (u.u_error)
			goto fail;
		iput(dp);
		goto null;
	}
	
fail:
	iput(dp);
noiputfail:
	p->dp = dp;
	return(NI_FAIL);

null:
	return(NI_NULL);

done:
	p->dp = dp;
	return(NI_DONE);

pass:
	p->dp = dp;
	p->ino = dir.d_ino;
	if (p->ino == S5ROOTINO)
		p->flags |= NX_ISROOT;
	return(NI_PASS);

#ifdef IFLNK
symlink:
	/* do everything that is necessary to process a symlink */
	/* (1) read in the direct block to get the real name. */
	/* (2) Prepend the name to the current component */
	/* (3) return(NI_RESTART) */
	return(NI_RESTART);
#endif
}

s5setattr(ip, flagp)
register struct inode *ip;
register struct argnamei *flagp;
{
	register struct s5inode *s5ip;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);

	if (u.u_uid != ip->i_uid && !suser())
		return(0);
	if (ip->i_mntdev->m_flags & MRDONLY) {
		u.u_error = EROFS;
		return(0);
	}

	switch (flagp->cmd) {
	case NI_CHMOD:
		s5ip->s5i_mode &= ~MODEMSK;
		if (u.u_uid) {
			flagp->mode &= ~ISVTX;
			if (u.u_gid != ip->i_gid)
				flagp->mode &= ~ISGID;
		}
		s5ip->s5i_mode |= flagp->mode&MODEMSK;
		if ((ip->i_flag & ITEXT) && (s5ip->s5i_mode & ISVTX) == 0)
			xrele(ip);
		return(1);

	case NI_CHOWN:
		if (u.u_uid != 0)
			s5ip->s5i_mode &= ~(ISUID|ISGID);
		ip->i_uid = flagp->uid;
		ip->i_gid = flagp->gid;
		return(1);

	}
	return(0);
}

/* ARGSUSED */
long
s5notify(ip, noargp)
register struct inode *ip;
register struct argnotify *noargp;
{
	ASSERT(noargp != NULL);
	switch ((int) noargp->cmd) {
	case NO_SEEK:
		if (noargp->data1 < 0)
			u.u_error = EINVAL;
		return(noargp->data1);
		break;
	}
	return(0L);
}

/*
 * This routine checks if a directory block has any non-trivial entries.
 * It returns -1 if it contains an entry different from either . or ..
 * and a flag indicating whether . and .. are
 * actually present otherwise.
 */

#define SDSIZ	(sizeof(struct direct))

emptblk(cp, size)
struct direct *cp;
int size;
{
	int n;
	register struct direct *dp;
	int flag = 0;

	dp = cp;
	for (n = size; n >= SDSIZ; n -= SDSIZ){
		if (dp ->d_ino != 0){
			if (dp->d_name[0] == '.' && dp->d_name[1] == '\0' )
				flag |= DOT;
			else if (dp->d_name[0] == '.' && dp->d_name[1] == '.' &&
			    dp->d_name[2] == '\0') 
				flag |= DOTDOT;
			else
				return(-1);
		}
		dp++;
	}
	return(flag);
}
