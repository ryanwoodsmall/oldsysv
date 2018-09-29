/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/io.h	10.1"

/*
 *	#include "sys/buf.h"
 *
 *	Each buffer in the pool is usually doubly linked into 2 lists:
 *	the device with which it is currently associated (always)
 *	and also on a list of blocks available for allocation
 *	for other use (usually).
 *	The latter list is kept in last-used order, and the two
 *	lists are doubly linked to make it easy to remove
 *	a buffer from one list when it was found by
 *	looking through the other.
 *	A buffer is on the available list, and is liable
 *	to be reassigned to another disk block, if and only
 *	if it is not marked BUSY.  When a buffer is busy, the
 *	available-list pointers can be used for other purposes.
 *	Most drivers use the forward ptr as a link in their I/O active queue.
 *	A buffer header contains all the information required to perform I/O.
 */

/*	The following is now a configurable parameter in the kernel
**	so define it here for lboot.
*/

#define	NOFILE	20

struct	buf
{
	int	b_flags;		/* see defines below */
	struct	buf *b_forw;		/* headed by bfreelist */
	struct	buf *b_back;		/*  "  */
	struct	buf *av_forw;		/* position on free list, */
	struct	buf *av_back;		/*     if not BUSY*/
	unsigned b_bcount;		/* transfer count */
	union {
	    caddr_t b_addr;		/* low order core address */
	    int	*b_words;		/* words for clearing */
	    struct filsys *b_filsys;	/* superblocks */
	    struct dinode *b_dino;	/* ilist */
	    daddr_t *b_daddr;		/* indirect block */
	} b_un;

	daddr_t	b_blkno;		/* block # on device */
	char	b_error;		/* returned after I/O */
};

/*
 *	These flags are kept in b_flags.
 */
#define B_READ    0x0001	/* read when I/O occurs */
#define B_DONE    0x0002	/* transaction finished */
#define B_ERROR   0x0004	/* transaction aborted */
#define B_BUSY    0x0008	/* not on av_forw/back list */
#define B_AGE     0x0080	/* delayed write for correct aging */
#define B_STALE   0x0800

/*
 *	Fast access to buffers in cache by hashing.
 */

#define bhash(b)	((struct buf *)&hbuf[((int)b) & (NHBUF-1)])

struct	hbuf
{
	int	b_flags;
	struct	buf	*b_forw;
	struct	buf	*b_back;
};

/*
 *	#include "sys/file.h"
 *
 * One file structure is allocated for each open.
 * Main use is to hold the read pointer associated with
 * each open file.
 *
 * For diagnostic purposes, the file name is remembered -- this is
 * an LBOOT extension.
 */

struct	file
{
	union {
		struct inode *f_uinode;	/* pointer to inode structure */
		struct file  *f_unext;	/* pointer to next entry in freelist */
	} f_up;
	off_t	f_offset;		/* read character pointer */

					/* LBOOT extension:	*/
	char	*f_name;		/*	remember the file name for diagnostic purposes */
};

#define f_inode		f_up.f_uinode
#define f_next		f_up.f_unext

/*
 * LBOOT's user structure
 *
 *	#include "sys/user.h"
 */

struct	user
{
	char	u_error;		/* return error code */

	caddr_t	u_base;			/* base address for IO */
	unsigned u_count;		/* bytes remaining for IO */
	off_t	u_offset;		/* offset in file for IO */
	ushort	u_pbsize;		/* Bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */

	struct inode *u_cdir;		/* pointer to inode of current directory */

	struct direct u_dent;		/* current directory entry */

	struct file *u_ofile[NOFILE];	/* pointers to file structures of open files */
};

#if DEBUG1f
/*
 * System activity statistics; only maintained if DEBUG1f is defined
 */

struct	sysinfo
	{
	long	bread;		/* blocks read from disk */
	long	lread;		/* logical blocks read from disk */
	long	bhit;		/* buffer cache hits */
	long	bmiss;		/* buffer cache misses */
	long	sysstat;
	long	sysopen;
	long	sysread;
	long	sysseek;
	long	sysclose;
	long	iget;
	long	ihit;		/* inode hash hits */
	long	imiss;		/* inode hash misses */
	long	namei;
	long	dirblk;		/* directory blocks read beyond first */
	long	readch;		/* total characters read */
	};

extern struct sysinfo sysinfo;
#endif
