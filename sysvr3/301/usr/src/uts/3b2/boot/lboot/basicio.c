/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/basicio.c	10.3"

/*
 * Notes on this file.
 *
 * This file is the UNIX 5.0 file system code for the 3B5.  It has been
 * extensively modified, BUT it has been done in a systematic manner.  This
 * note is an attempt to give an overview of the modifications.  Hopefully,
 * if it is desired to re-create this file by again copying the file system
 * code, then this note will provide either a historical perspective or a
 * starting point.
 *
 * First of all, this code is pieces of the following files in the kernel,
 * and header files:
 *
 *		os/fio.c	: getf, closef, openi, ufalloc, falloc, finit
 *		os/iget.c	: inoinit, iget, iread, iput
 *		os/pipe.c	: plock, prele
 *		os/main.c	: main(renamed fsinit), iinit, binit
 *		os/nami.c	: namei
 *		os/rdwri.c	: readi, bmap
 *		os/sys2.c	: open, read, lseek, close
 *		os/sys3.c	: stat, stat1
 *		os/sys4.c	: chdir
 *		io/bio.c	: bread, brelse, getblk, geteblk, clrbuf
 *		sys/buf.h	==> see io.h
 *		sys/file.h	==> see io.h
 *		sys/user.h	==> see io.h
 *		sys/space.h
 *
 * Secondly, the reason for copying the UNIX code is to get a rudimentary
 * file system up and running with the least amount of work.  In addition,
 * there is some reason to believe that the code will work, since it was
 * copied, and not written from scratch.  The objective, then, is to copy
 * the code, make as few modifications as necessary, and use it in the boot
 * program to provide the normal UNIX style interface to the disk.  This
 * objective is the reason that there is a ublock (which is silly, but
 * it means fewer changes to the code), a buffer cache, inode and file
 * tables, etc.
 *
 * Lastly, the following steps were taken to effect the modifications.
 *
 *	1. Concatenate all of the kernel files.
 *	2. Move all of the #includes to the beginning of the file.
 *	3. Replace those #includes which contain too much kernel
 *	   dependent stuff with copies, and remove everything that
 *	   is not applicable to simply reading a file system.  The
 *	   hashing functions were also changed here.
 *	4. Remove ondly, the reason for copying the UNIX code is to get a rudimentary
 * file system up and running with the least amount of work.  In addition,
 * there is some reason to believe that the code will work, since it was
 * copied, and not written from scratch.  The objective, then, is to copy
 * the code, make as few modifications as necessary, and use it in the boot
 * program to provide the normal UNIX style interface to the disk.  This
 * objective is the reason that there is a ublock (which is silly, but
 * it means fewer changes to the code), a buffer cache, inode and file
 * tables, etc.
 *
 * Lastly, the following steps were taken to effect the modifications.
 *
 *	1. Concatenate all of the kernel files.
 *	2. Move all of the #includes to the beginning of the file.
 *	3. Replace those #includes which contain too much kernel
 *	   dependent stuff with copies, and remove everything that
 *	   is not applicable to simply reading a file system.  The
 *	   hashing functions were also changed here.
 *	4. Remove all routines and pieces of routines that have nothing
 *	   to do with reading a file system.  Thus, any code or routines
 *	   which have to do with updating or writing the disk is removed.
 *	5. All code that deals with the device number (dev_t) is
 *	   replaced with a reference to the variable `root'.  This
 *	   is simply because we are dealing with one and only one file
 *	   system.  This also means that any code dealing with the mount
 *	   table can be either removed or replaced by a single reference
 *	   to `root'.
 *	6. All spl routine calls were removed.
 *	7. All code for read-ahead was removed.
 *	8. Anything associated with asynchronous read was removed.
 *
 * Now that this overview is complete, one should be aware that the "few
 * modifications" will require a good part of 2-3 days.  A final hint:
 * use lint; it is an invaluable aid.
 */


#include <sys/types.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/param.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include "io.h"
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/*
 * If INCORE is defined, then the buffer cache will expand to the size of
 * mainstore.  If INCORE is not defined, then the number of buffers is
 * fixed by the define'd constant NBUF.
 */
#define	INCORE

/*
 *	#include "sys/space.h"
 */
#ifndef INCORE
#if TEST
#define NBUF	10
#else
#define NBUF	25
#endif

#define NHBUF	4		/* <==must be a power of 2 */

#else

#define	NHBUF	32		/* <==must be a power of 2 */
#endif

#define NFILE	NOFILE
#define NINODE	NFILE+3		/* few more inodes than files */

struct	buf	bfreelist;	/* head of the free list of buffers */

#ifndef INCORE
struct	buf	buf[NBUF];	/* buffer headers */
#ifdef u3b5
char	buffers	[NBUF][SBUFSIZE];	/* buffer cache */
#endif
#ifdef u3b2
char	buffers	[NBUF+1][SBUFSIZE];	/* buffer cache */
#endif
#else ! INCORE
struct	buf	*buf;		/* acquired buffer headers */
#endif

struct	hbuf	hbuf[NHBUF];	/* buffer hash table */

struct	file	file[NFILE];	/* file table */
struct	file	*ffreelist;

struct	inode	inode[NINODE];	/* inode table */
struct ifreelist	ifreelist;

#define	NHINO	4		/* <==must be a power of 2 */
#define ihash(X)	(&hinode[(int) (X) & (NHINO-1)])

struct	hinode	{
	struct	inode	*i_forw;
	struct	inode	*i_back;
	} hinode[NHINO];



#if DEBUG1f
/*
 * System statistics
 */

struct sysinfo sysinfo;
#endif


/*
 * Dummy user area
 */
struct user u;

/*
 * Important variables
 *
 *	superblock -> struct buf for the file system superblock
 *	root    -- the pseudo-device for the boot disk; it is only
 *	              used to determine the file system size
 *	rootdir    -> struct inode of the root directory
 */
struct buf	*superblock;
dev_t		root;
struct inode	*rootdir;

/*
 * Static function declarations for this file
 */
static daddr_t          bmap();
static struct buf      *bread();
static struct file     *falloc();
static struct buf      *getblk();
static struct buf      *geteblk();
static struct file     *getf();
static struct inode    *iget();
static struct inode    *iread();
static struct inode    *namei();

/*
 *	 Copied from ../../os/sys2.c 
 */

/*
 * open system call
 *
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 *
 * For LBOOT, set f_name to remember the file name.
 */
open(fname)
char	*fname;
{PROGRAM(open)
	register struct inode *ip;
	register struct file *fp;
	register int i;

#if DEBUG1f
	++sysinfo.sysopen;
#endif
	u.u_error = 0;
	ip = namei(fname);
	if (ip == NULL)
		{
		errno = u.u_error;
		return(-1);
		}
	if (u.u_error || (fp = falloc(ip)) == NULL) {
		iput(ip);
		errno = u.u_error;
		return(-1);
	}
	prele(ip);
	openi(ip);
	for (i=0; i<NFILE; ++i)
		if (u.u_ofile[i] == fp)
			{
			fp->f_name = fname;	/* remember file name */
			if (u.u_error == 0)
				return(i);
			u.u_ofile[i] = NULL;
			break;
			}
	fp->f_next = ffreelist;
	ffreelist = fp;
	iput(ip);
	errno = u.u_error;
	return(-1);
}

/*
 * read system call
 *
 * check permissions, set base, count, and offset,
 * and switch out to readi.
 */
read(fdes,cbuf,count)
int	fdes;
char	*cbuf;
unsigned count;
{PROGRAM(read)
	register struct file *fp;
	register struct inode *ip;
	register int	type;

#if DEBUG1f
	++sysinfo.sysread;
#endif
	u.u_error = 0;
	fp = getf(fdes);
	if (fp == NULL)
		{
		errno = u.u_error;
		return(-1);
		}
	u.u_base = (caddr_t)cbuf;
	u.u_count = count;
	ip = fp->f_inode;
	type = ip->i_mode&IFMT;
	if (type==IFREG || type==IFDIR) {
		plock(ip);
	}
	u.u_offset = fp->f_offset;
	readi(ip);
	if (type==IFREG || type==IFDIR)
		prele(ip);
	fp->f_offset += count-u.u_count;
#if DEBUG1f
	sysinfo.readch += count-u.u_count;
#endif
	if (u.u_error == 0)
		return(count - u.u_count);
	errno = u.u_error;
	return(-1);
}

/*
 * seek system call
 */
 off_t
lseek(fdes,off,sbase)
int	fdes;
register off_t	off;
register int	sbase;
{PROGRAM(lseek)
	register struct file *fp;

#if DEBUG1f
	++sysinfo.sysseek;
#endif
	u.u_error = 0;
	fp = getf(fdes);
	if (fp == NULL)
		{
		errno = u.u_error;
		return(-1);
		}
	if (sbase == 1)
		off += fp->f_offset;
	else if (sbase == 2)
		off += fp->f_inode->i_size;
	else if (sbase != 0) {
		u.u_error = EINVAL;
		errno = u.u_error;
		return(-1);
	}
	if (off < 0) {
		u.u_error = EINVAL;
		errno = u.u_error;
		return(-1);
	}
	fp->f_offset = off;
	return(off);
}

/*
 * close system call
 */
 void
close(fdes)
register int	fdes;
{PROGRAM(close)
	register struct file *fp;

#if DEBUG1f
	++sysinfo.sysclose;
#endif
	u.u_error = 0;
	fp = getf(fdes);
	if (fp == NULL)
		return;
	u.u_ofile[fdes] = NULL;
	closef(fp);
}

/*
 *	 New for LBOOT; access saved file name of opened file
 */


/*
 * *NEW* filename system call
 */
 char *
filename(fdes)
int	fdes;
{PROGRAM(filename)
	register struct file *fp;

	u.u_error = 0;
	fp = getf(fdes);
	if (fp == NULL)
		{
		errno = u.u_error;
		return(NULL);
		}
	return(fp->f_name);
}

/*
 *	 Copied from ../../os/sys3.c 
 */

/*
 * the stat system call.
 */
stat(fname,sb)
char	*fname;
struct stat *sb;
{PROGRAM(stat)
	register struct inode *ip;
	register rc;

	u.u_error = 0;
	ip = namei(fname);
	if (ip == NULL)
		{
		errno = u.u_error;
		return(-1);
		}
	rc = stat1(ip, sb);
	iput(ip);
	return(rc);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
 static
stat1(ip, ub)
register struct inode *ip;
register struct stat *ub;
{PROGRAM(stat1)
	register struct dinode *dp;
	register struct buf *bp;

#if DEBUG1f
	++sysinfo.sysstat;
#endif
	/*
	 * first copy from inode table
	 */
	ub->st_dev = root;
	ub->st_ino = ip->i_number;
	ub->st_mode = ip->i_mode;
	ub->st_nlink = ip->i_nlink;
	ub->st_uid = ip->i_uid;
	ub->st_gid = ip->i_gid;
	ub->st_rdev = ip->i_rdev;
	ub->st_size = ip->i_size;
	/*
	 * next the dates in the disk
	 */
	bp = bread(FsITOD(root, ip->i_number));
	dp = bp->b_un.b_dino;
	dp += FsITOO(root, ip->i_number);
	ub->st_atime = dp->di_atime;
	ub->st_mtime = dp->di_mtime;
	ub->st_ctime = dp->di_ctime;
	brelse(bp);
	if (u.u_error == 0)
		return(0);
	else
		{
		errno = u.u_error;
		return(-1);
		}
}

/*
 *	 Copied from ../../os/sys4.c 
 */

chdir(fname)
char *fname;
{PROGRAM(chdir)
	register struct inode *ip;

	u.u_error = 0;
	ip = namei(fname);
	if (ip == NULL)
		{
		errno = u.u_error;
		return(-1);
		}
	if ((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	prele(ip);
	if (u.u_cdir) {
		plock(u.u_cdir);
		iput(u.u_cdir);
	}
	u.u_cdir = ip;
	return(0);

bad:
	iput(ip);
	errno = u.u_error;
	return(-1);
}

/*
 *	 Copied from ../../os/fio.c 
 */


/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.
 * Only task is to check range of the descriptor.
 */
 static
 struct file *
getf(f)
register int f;
{PROGRAM(getf)
	register struct file *fp;

	if (0 <= f && f < NOFILE) {
		fp = u.u_ofile[f];
		if (fp != NULL)
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 */
 static
closef(fp)
register struct file *fp;
{PROGRAM(closef)
	register struct inode *ip;

	if (fp == NULL)
		return;
	ip = fp->f_inode;
	fp->f_next = ffreelist;
	ffreelist = fp;
	iput(ip);
}

/*
 * openi called to allow handler of special files to initialize and
 * validate before actual IO.
 */
 static
openi(ip)
register struct inode *ip;
{PROGRAM(openi)
	switch (ip->i_mode&IFMT) {

	case IFCHR:
	case IFBLK:
	case IFIFO:
		u.u_error = ENXIO;
	}
}

/*
 * Allocate a user file descriptor.
 */
 static
ufalloc()
{PROGRAM(ufalloc)
register i;

	for (i=0; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor and a file structure.
 * Initialize the descriptor to point at the file structure.
 *
 * no file -- if there are no available file structures.
 */
 static
 struct file *
falloc(ip)
struct inode *ip;
{PROGRAM(falloc)
	register struct file *fp;
	register i;

	i = ufalloc();
	if (i < 0)
		return(NULL);
	if ((fp=ffreelist) == NULL) {
		panic("file table overflow");
	}
	ffreelist = fp->f_next;
	u.u_ofile[i] = fp;
	fp->f_inode = ip;
	fp->f_offset = 0;
	return(fp);
}

 static
finit()
{PROGRAM(finit)
	register struct file *fp;

	for (ffreelist = fp = &file[0]; fp < &file[NFILE-1]; fp++)
		fp->f_next = fp+1;
}

/*
 *	 Copied from ../../os/iget.c 
 */


/*
 * initialize inodes
 */
 static
inoinit()
{PROGRAM(inoinit)
	register struct inode *ip;
	register int i;

	ifreelist.av_forw = ifreelist.av_back = (struct inode *) &ifreelist;
	for (i = 0; i < NHINO; i++)
		hinode[i].i_forw=hinode[i].i_back=(struct inode *) &hinode[i];
	for (i = 0, ip = inode; i < NINODE; i++, ip++) {
		ip->i_forw = ip->i_back = ip;
		(ifreelist.av_forw)->av_back = ip;
		ip->av_forw = ifreelist.av_forw;
		ifreelist.av_forw = ip;
		ip->av_back = (struct inode *) &ifreelist;
	}
}

/*
 * Look up an inode by inumber.
 * If it is in core (in the inode structure), honor the locking protocol.
 * If it is not in core, read it in from the specified device.
 * In all cases, a pointer to a locked inode structure is returned.
 */

 static
 struct inode *
iget(ino)
register ino_t ino;
{PROGRAM(iget)
	register struct inode *ip;
	register struct hinode *hip;

#if DEBUG1f
	++sysinfo.iget;
#endif
	hip = ihash(ino);
	for (ip = hip->i_forw; ip != (struct inode *) hip; ip = ip->i_forw)
		if (ino == ip->i_number)
			goto found;

	if ((ip = ifreelist.av_forw) == NULL) {
		panic("inode table overflow");
	}

#if DEBUG1f
	++sysinfo.imiss;
#endif
	ip->av_back->av_forw = ip->av_forw;	/* remove from free list */
	ip->av_forw->av_back = ip->av_back;

	ip->i_back->i_forw = ip->i_forw;	/* remove from old hash list */
	ip->i_forw->i_back = ip->i_back;

	hip->i_forw->i_back = ip;	/* insert into new hash list */
	ip->i_forw = hip->i_forw;
	hip->i_forw = ip;
	ip->i_back = (struct inode *) hip;

	ip->i_number = ino;
	ip->i_flag = ILOCK;
	ip->i_count = 1;
	return(iread(ip));
found:
#if DEBUG1f
	++sysinfo.ihit;
#endif
	if ((ip->i_flag&ILOCK) != 0) {
		panic("inode locked");
	}
	if (ip->i_count == 0) {
				/* remove from freelist */
		ip->av_back->av_forw = ip->av_forw;
		ip->av_forw->av_back = ip->av_back;
	}
	ip->i_count++;
	ip->i_flag |= ILOCK;
	return(ip);
}

 static
 struct inode *
iread(ip)
register struct inode *ip;
{PROGRAM(iread)
	register char *p1, *p2;
	register struct dinode *dp;
	register struct buf *bp;
	register i;

	bp = bread(FsITOD(root, ip->i_number));
	if (u.u_error) {
		brelse(bp);
		iput(ip);
		return(NULL);
	}
	dp = bp->b_un.b_dino;
	dp += FsITOO(root, ip->i_number);
	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	for (i=0; i<NADDR; i++) {
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
	brelse(bp);
	return(ip);
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, free the inode
 */
 static
iput(ip)
register struct inode *ip;
{PROGRAM(iput)
	if (ip->i_count == 1) {
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
 *	 Copied from ../../os/pipe.c 
 */


/*
 * Lock a pipe.
 * If its already locked,
 * set the WANT bit and sleep.
 */
 static
plock(ip)
	register struct inode *ip;
{PROGRAM(plock)

	if (ip->i_flag&ILOCK) {
		panic("inode locked");
	}
	ip->i_flag |= ILOCK;
}

/*
 * This routine is also used
 * to unlock inodes in general.
 */
 static
prele(ip)
	register struct inode *ip;
{PROGRAM(prele)
	ip->i_flag &= ~ILOCK;
}

/*
 *	 Copied from ../../os/nami.c 
 */

/*
 * Convert a pathname into a pointer to
 * an inode. Note that the inode is locked.
 */
 static
 struct inode *
namei(fname)
register char *fname;
{PROGRAM(namei)
	register struct inode *dp;
	register c;
	register char *cp;
	register struct buf *bp;
	register i;

#if DEBUG1f
	++sysinfo.namei;
#endif
	/*
	 * If name starts with '/' start from
	 * root; otherwise start from current dir.
	 */

	c = *fname++;
	if (c == '\0') {
		u.u_error = ENOENT;
		return(NULL);
	}
	if (c == '/') {
		dp = rootdir;
		while (c == '/')
			c = *fname++;
	} else
		dp = u.u_cdir;
	(void) iget(dp->i_number);

cloop:
	/*
	 * Here dp contains pointer
	 * to last component matched.
	 */

	if (u.u_error)
		goto out;
	if (c == '\0')
		return(dp);

	/*
	 * If there is another component,
	 * gather up name into users' dir buffer.
	 */


	cp = &u.u_dent.d_name[0];
	while (c != '/' && c != '\0' && u.u_error == 0) {
		if (cp < &u.u_dent.d_name[DIRSIZ])
			*cp++ = c;
		c = *fname++;
	}
	while (cp < &u.u_dent.d_name[DIRSIZ])
		*cp++ = '\0';
	while (c == '/')
		c = *fname++;

	/*
	 * dp must be a directory and
	 * must have X permission.
	 */
	if ((dp->i_mode&IFMT) != IFDIR || dp->i_nlink==0)
		u.u_error = ENOTDIR;
	if (u.u_error)
		goto out;

	/*
	 * set up to search a directory
	 */
	u.u_offset = 0;
	u.u_count = dp->i_size;
	u.u_pbsize = 0;
	bp = NULL;
	if (dp == rootdir)
		if (u.u_dent.d_name[0] == '.')
			if (u.u_dent.d_name[1] == '.')
				if (u.u_dent.d_name[2] == '\0')
					goto cloop;

eloop:

	/*
	 * If at the end of the directory,
	 * the search failed.
	 */

	if (u.u_offset >= dp->i_size) {
		if (bp != NULL)
			brelse(bp);
		u.u_error = ENOENT;
		goto out;
	}

	/*
	 * If offset is on a block boundary,
	 * read the next directory block.
	 * Release previous if it exists.
	 */

	if (u.u_pbsize == 0) {
		daddr_t bn;

		if (bp != NULL)
			brelse(bp);
#if DEBUG1f
		++sysinfo.dirblk;
#endif
		bn = bmap(dp);
		if (u.u_error)
			goto out;
		if (bn < 0) {
			u.u_error = EIO;
			goto out;
		}
		bp = bread(bn);
		if (u.u_error) {
			brelse(bp);
			goto out;
		}
	}

	/*
	 * String compare the directory entry
	 * and the current component.
	 * If they do not match, go back to eloop.
	 */

	cp = bp->b_un.b_addr + u.u_pboff;
	u.u_offset += sizeof(struct direct);
	u.u_pboff += sizeof(struct direct);
	u.u_pbsize -= sizeof(struct direct);
	u.u_count -= sizeof(struct direct);
	u.u_dent.d_ino = ((struct direct *)cp)->d_ino;
	if (u.u_dent.d_ino == 0)
		goto eloop;
	cp = &((struct direct *)cp)->d_name[0];
	for (i=0; i<DIRSIZ; i++)
		if (*cp++ != u.u_dent.d_name[i])
			goto eloop;

	/*
	 * Here a component matched in a directory.
	 * If there is more pathname, go back to
	 * cloop, otherwise return.
	 */

	if (bp != NULL)
		brelse(bp);
	iput(dp);
	dp = iget(u.u_dent.d_ino);
	if (dp == NULL)
		return(NULL);
	goto cloop;

out:
	iput(dp);
	return(NULL);
}

/*
 *	 Copied from ../../os/rdwri.c 
 */

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 * The i_mode must be one of IFBLK,IFDIR or IFREG
 */
 static
readi(ip)
register struct inode *ip;
{PROGRAM(readi)
	register struct buf *bp;
	register daddr_t bn;
	register unsigned on, n;
	register type;

	if (u.u_count == 0)
		return;
	if (u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	type = ip->i_mode&IFMT;
	switch (type) {
	case IFCHR:
		u.u_error = EINVAL;
		break;

	case IFBLK:
	case IFREG:
	case IFDIR:
	do {
		bn = bmap(ip);
		if (u.u_error)
			break;
		on = u.u_pboff;
		if ((n = u.u_pbsize) == 0)
			break;
		if ((long)bn<0) {
			if (type != IFREG)
				break;
			bp = geteblk();
			clrbuf(bp);
		} else
			bp = bread(bn);
		if (u.u_error == 0) {
			bcopy(bp->b_un.b_addr+on, u.u_base, n);

			u.u_offset += n;
			u.u_count -= n;
			u.u_base += n;
		}
		brelse(bp);
	} while (u.u_error==0 && u.u_count!=0);
		break;

	default:
		u.u_error = ENODEV;
	}
}

/*
 *	 Copied from ../../os/subr.c 
 */


/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
 static
 daddr_t
bmap(ip)
register struct inode *ip;
{PROGRAM(bmap)
	register daddr_t bn;

	{
		register sz, rem, type;

		type = ip->i_mode&IFMT;
		bn = u.u_offset >> FsBSHIFT(root);
		if (bn < 0) {
			u.u_error = EFBIG;
			return((daddr_t)-1);
		}
		u.u_pboff = u.u_offset & FsBMASK(root);
		sz = FsBSIZE(root) - u.u_pboff;
		if (u.u_count < sz) {
			sz = u.u_count;
		}
		u.u_pbsize = sz;
		if (type == IFBLK) {
			return(bn);
		}
		rem = ip->i_size - u.u_offset;
		if (rem < 0)
			rem = 0;
		if (rem < sz)
			sz = rem;
		if ((u.u_pbsize = sz) == 0)
			return((daddr_t)-1);
	}

	{
		register daddr_t nb;
		register struct buf *bp;
		register i, j, sh;
		daddr_t *bap;
		int nshift;

		/*
		 * blocks 0..NADDR-4 are direct blocks
		 */
		if (bn < NADDR-3) {
			i = bn;
			nb = ip->i_addr[i];
			if (nb == 0) {
				return((daddr_t)-1);
			}
			return(nb);
		}

		/*
		 * addresses NADDR-3, NADDR-2, and NADDR-1
		 * have single, double, triple indirect blocks.
		 * the first step is to determine
		 * how many levels of indirection.
		 */
		nshift = FsNSHIFT(root);
		sh = 0;
		nb = 1;
		bn -= NADDR-3;
		for (j=3; j>0; j--) {
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
		nb = ip->i_addr[NADDR-j];
		if (nb == 0) {
			return((daddr_t)-1);
		}

		/*
		 * fetch through the indirect blocks
		 */
		for (; j<=3; j++) {
			bp = bread(nb);
			if (u.u_error) {
				brelse(bp);
				return((daddr_t)-1);
			}
			bap = bp->b_un.b_daddr;
			sh -= nshift;
			i = (bn>>sh) & FsNMASK(root);
			nb = bap[i];
			if (nb == 0) {
				brelse(bp);
				return((daddr_t)-1);
				}
			brelse(bp);
		}
		return(nb);
	}
}

/*
 *	 Copied from ../../io/bio.c 
 */

/*
 * Unlink a buffer from the HEAD of the available list and mark it busy.
 * (internal interface)
 */
#define notavail(bp) \
{\
	bp->av_back->av_forw = bp->av_forw;\
	bp->av_forw->av_back = bp->av_back;\
	bp->b_flags |= B_BUSY;\
	bfreelist.b_bcount--;\
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized code
 */
#define geterror(bp) \
{\
\
	if (bp->b_flags&B_ERROR)\
		if ((u.u_error = bp->b_error)==0)\
			u.u_error = EIO;\
}

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, we will panic.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 * Eventually the buffer must be released by using
 *	brelse
 */

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
 static
 struct buf *
bread(blkno)
daddr_t blkno;
{PROGRAM(bread)
	register struct buf *bp;

#if DEBUG1f
	++sysinfo.lread;
#endif
	bp = getblk(blkno);
	if (bp->b_flags&B_DONE)
		return(bp);
	bp->b_flags |= B_READ;
	bp->b_bcount = FsBSIZE(root);
#if DEBUG1f
	++sysinfo.bread;
#endif
	dfcread(bp);
	geterror(bp);
	return(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
 static
brelse(bp)
register struct buf *bp;
{PROGRAM(brelse)
	register struct buf **backp;

	if (bp->b_flags&B_ERROR) {
		bp->b_flags |= B_STALE|B_AGE;
		bp->b_flags &= ~(B_ERROR);
		bp->b_error = 0;
	}
	if (bp->b_flags & B_AGE)
		{
		/*
	 	 * return buffer to HEAD of available chain
		 */
		backp = &bfreelist.av_forw;
		(*backp)->av_back = bp;
		bp->av_forw = *backp;
		*backp = bp;
		bp->av_back = &bfreelist;
		}
	else
		{
		/*
	 	 * return buffer to TAIL of available chain
		 */
		backp = &bfreelist.av_back;
		(*backp)->av_forw = bp;
		bp->av_back = *backp;
		*backp = bp;
		bp->av_forw = &bfreelist;
		}
	bp->b_flags &= ~(B_BUSY|B_AGE);
	bfreelist.b_bcount++;
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 */
 static
 struct buf *
getblk(blkno)
register daddr_t blkno;
{PROGRAM(getblk)
	register struct buf *bp;
	register struct buf *dp;

	blkno = FsLTOP(root, blkno);
	dp = bhash(blkno);
	if (dp == NULL)
		panic("devtab");
	for (bp=dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_blkno!=blkno || bp->b_flags&B_STALE)
			continue;
		if (bp->b_flags&B_BUSY) {
			panic("block busy");
		}
		notavail(bp);
#if DEBUG1f
		++sysinfo.bhit;
#endif
		return(bp);
	}
	if (bfreelist.av_forw == &bfreelist) {
		panic("out of free blocks");
	}
	bp = bfreelist.av_forw;
	notavail(bp);			/* remove from the available list */

	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;	/* remove from any device chain */
	bp->b_forw->b_back = bp->b_back;

	bp->b_forw = dp->b_forw;		/* put on hbuf chain */
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;

	bp->b_blkno = blkno;
	bp->b_bcount = FsBSIZE(root);
#if DEBUG1f
	++sysinfo.bmiss;
#endif
	return(bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
 static
 struct buf *
geteblk()
{PROGRAM(geteblk)
	register struct buf *bp;
	register struct buf *dp;

	if (bfreelist.av_forw == &bfreelist) {
		panic("out of free blocks");
	}
	dp = &bfreelist;
	bp = bfreelist.av_forw;
	notavail(bp);			/* remove from the available list */

	bp->b_flags = B_BUSY|B_AGE;
	bp->b_back->b_forw = bp->b_forw;	/* remove from any device chain */
	bp->b_forw->b_back = bp->b_back;

	bp->b_forw = dp->b_forw;		/* put on bfreelist chain */
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;

	bp->b_bcount = SBUFSIZE;
	return(bp);
}

/*
 * Zero the core associated with a buffer.
 */
 static
clrbuf(bp)
register struct buf *bp;
{PROGRAM(clrbuf)
	bzero(bp->b_un.b_words, bp->b_bcount);
}

/*
 *	 Copied from ../../os/main.c 
 */

/*
 * Initialization code.
 * Functions:
 *	clear and free user core
 *	call all initialization routines
 *
 * Returns:	TRUE  - success
 *		FALSE - failure
 */
 boolean
fsinit()
	{PROGRAM(fsinit)

	/*
	 * initialize system tables and i/o drivers
	 */

	binit();
	inoinit();
	finit();

	if (! dfinit())
		return(FALSE);	/* Return to IAU - INIT FAILED! */

	iinit(); /* read in SUPERBLOCK and determine blocksize */
	
	/*
	 * Set up access to root file system.
	 */
	rootdir = iget(ROOTINO);

	if (rootdir == NULL)
		return(FALSE);	/* Return to IAU */

	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;

	return(TRUE);
}

/*
 *	iinit is called once very early in initialization.
 *	It reads the root's super block
 *
 */
 static
iinit()
{PROGRAM(iinit)
	register struct buf *cp;
	register struct filsys *fp;
	struct inode iinode;

	cp = geteblk();
	fp = cp->b_un.b_filsys;
	iinode.i_mode = IFBLK;
	u.u_error = 0;
	u.u_offset = SUPERBOFF;
	u.u_count = sizeof(struct filsys);
	u.u_base = (caddr_t) fp;
	readi(&iinode);
	if (u.u_error)
		panic("cannot mount root");
	if (fp->s_magic != FsMAGIC)
		fp->s_type = Fs1b;
	if (fp->s_type == Fs2b)
		root |= Fs2BLK;
	superblock = cp;
}

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device hash buffer lists to empty.
 */
 static
binit()
{PROGRAM(binit)
	register struct buf *bp;
	register struct buf *dp;
	register unsigned i;
	register caddr_t pbuffer;
#ifdef INCORE

#define	K	1024L

	int NBUF;
	extern address maxmem, topboot;
	extern int end[];


	/* use maximum of 1 megabyte */
	NBUF = min(K*K, (long)(maxmem - (address)end)) / (sizeof(struct buf) + SBUFSIZE);

#if DEBUG1
	if (prt[_BASICIO])
		{
		char answer[81], *p;

		printf("Binit: NBUF=      (default %d)\rBinit: NBUF=", NBUF);
		if (scanf(answer) == SCANF_OK && (p=strtok(answer,"\r\n\t ")) != NULL)
			NBUF = strtol(p, (char**)NULL, 0);
		}
#endif

#ifdef u3b2
	NBUF -= 1;
#endif
#endif

	dp = &bfreelist;
	dp->b_forw = dp->b_back =
	    dp->av_forw = dp->av_back = dp;

#ifndef INCORE
#ifdef u3b5
	pbuffer = (caddr_t)buffers;
#endif
#ifdef u3b2
	/*
	 * buffers must not cross 64K boundry on 3B2, therefore align the
	 * buffer pool on a BSIZE boundry
	 */
	pbuffer = (caddr_t)(((int)buffers + BSIZE-1) & ~BMASK);
#endif
#else ! INCORE
	buf = (struct buf*)end;
#ifdef u3b5
	pbuffer = (caddr_t)(buf+NBUF);
#endif
#ifdef u3b2
	/*
	 * buffers must not cross 64K boundry on 3B2, therefore align the
	 * buffer pool on a BSIZE boundry
	 */
	pbuffer = (caddr_t)(((int)(buf+NBUF) + BSIZE-1) & ~BMASK);
#endif

	topboot = (address)(pbuffer + SBUFSIZE*NBUF);
#if DEBUG1
	if (prt[_BASICIO])
		printf("Binit: topboot=0x%lX\n", topboot);
#endif
#endif

	for (i=0, bp=buf; i<NBUF; i++,bp++,pbuffer += SBUFSIZE) {
		bp->b_un.b_addr = pbuffer;
		bp->b_back = dp;
		bp->b_forw = dp->b_forw;
		dp->b_forw->b_back = bp;
		dp->b_forw = bp;
		bp->b_flags = B_BUSY;
		bp->b_bcount = 0;
		brelse(bp);
	}
	for (i=0; i < NHBUF; i++)
		hbuf[i].b_forw = hbuf[i].b_back = (struct buf *)&hbuf[i];
}

#if DEBUG1g
/*
 * Files()
 *
 * Print the status of the currently opened files and streams
 */
 void
files()
	{PROGRAM(files)

	register i;
	register struct file *fp;
	register FILE *stream;
	char buffer[50];
	extern FILE _iob[];

	printf("\nFiles open:\n");

	for (i=0; i<NOFILE; ++i)
		if ((fp=u.u_ofile[i]) != NULL)
			printf("     [%d]   %s   offset=%D\n", i, fp->f_name, fp->f_offset);

	printf("\nStreams open:\n");

	for (i=0; i<NOFILE; ++i)
		{
		stream = &_iob[i];

		if (stream->_flag)
			{
			buffer[0] = '\0';
			buffer[1] = '\0';

			if (stream->_flag & _IOREAD)
				strcat(buffer, ",_IOREAD");

			if (stream->_flag & _IOMYBUF)
				strcat(buffer, ",_IOMYBUF");

			if (stream->_flag & _IOEOF)
				strcat(buffer, ",_IOEOF");

			if (stream->_flag & _IOERR)
				strcat(buffer, ",_IOERR");

			printf("     0x%lX[%d]   file=%d   flag=%s\n", stream, i, fileno(stream), buffer+1);
			}
		}

	printf("\n");
	}
#endif
