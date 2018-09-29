/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/file.h	10.4"
/*
 * One file structure is allocated for each open/creat/pipe call.
 * Main use is to hold the read/write pointer associated with
 * each open file.
 */

typedef struct file
{
	char	f_flag;
	cnt_t	f_count;		/* reference count */
	union {
		struct inode *f_uinode;	/* pointer to inode structure */
		struct file  *f_unext;	/* pointer to next entry in freelist */
	} f_up;
	off_t	f_offset;		/* read/write character pointer */
} file_t;

#define f_inode		f_up.f_uinode
#define f_next		f_up.f_unext

extern struct file file[];	/* The file table itself */
extern struct file *ffreelist;	/* Head of freelist pool */

/* flags */

#define	FOPEN	0xffffffff
#define	FREAD	0x01
#define	FWRITE	0x02
#define	FNDELAY	0x04
#define	FAPPEND	0x08
#define FSYNC	0x10
#define	FMASK	0x7f		/* FMASK should be disjoint from FNET */
#define FNET	0x80		/* needed by 3bnet */

/* open only modes */

#define	FCREAT	0x100
#define	FTRUNC	0x200
#define	FEXCL	0x400
