/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/fs/s5inode.h	10.2"
#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

struct s5inode {
	ushort	s5i_flags;	/* see below */	
	ushort	s5i_mode;	/* file mode and type */
	union s5i_u1 {
		struct	s5inode	*s5i_nxt; /* free list pntr */
		daddr_t s5i_a[NADDR];	/* if normal file/directory */
		short	s5i_f[NSADDR];	/* counts for fifo's */
	} s5i_u1;
	union s5i_u2 {
		int	*s5i_mp;	/* pointer to blk # map for paging */
		off_t	s5i_doff;	/* directory offset - */
					/* for directory caching */
	} s5i_u2;
	daddr_t	s5i_l;		/* last logical block read (for read-ahead) */
};

/* Flags */
#define	S5IFREE		0x0	/* Free cell */

#define	s5i_next	s5i_u1.s5i_nxt
#define	s5i_addr	s5i_u1.s5i_a
#define	s5i_lastr	s5i_l
#define	s5i_rdev	s5i_u1.s5i_a[0]

#define	s5i_map		s5i_u2.s5i_mp
#define	s5i_diroff	s5i_u2.s5i_doff

#define	s5i_faddr	s5i_u1.s5i_a
#define	NFADDR	10
#define	PIPSIZ	NFADDR*BSIZE
#define	s5i_frptr	s5i_u1.s5i_f[NSADDR-5]
#define	s5i_fwptr	s5i_u1.s5i_f[NSADDR-4]
#define	s5i_frcnt	s5i_u1.s5i_f[NSADDR-3]
#define	s5i_fwcnt	s5i_u1.s5i_f[NSADDR-2]
#define	s5i_waite	s5i_u1.s5i_f[NSADDR-3]
#define	s5i_waitf	s5i_u1.s5i_f[NSADDR-2]
#define	s5i_fflag	s5i_u1.s5i_f[NSADDR-1]
#define	IFIR	01
#define	IFIW	02

extern struct s5inode s5inode[];
