/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/mount.h	10.10"
/*
 *	Mount structure.
 *	One allocated on every mount.
 */

#ifndef DIRSIZ				/* REMOTE */
#define DIRSIZ	14
#endif
#define	NMSZ	DIRSIZ + 1		/* size of advertised name */

struct	mount	{
	ushort	m_flags;		/* status */
	short	m_fstyp;		/* file system type */
	long    m_bsize;		/* Block size of this fs */
	dev_t	m_dev;			/* device mounted */
	caddr_t	m_bufp;			/* buffer for super block */
	union {
		struct inode *inodp;	/* pointer to mounted on inode */
		struct sndd  *upchan;	/* LBIN case, mounted remotely */
	} u1;
	union {
		struct inode *mountp;	/* pointer to mount root inode */
		struct sndd  *dnchan;	/* remote mount case, root elsewhere */
	} u2;
	char	m_rflags;		/* set to MLBIN for mount remotely  */
	char	*m_name;		/* name advertised resource */
	long	m_bcount;
} ;

#define m_inodp u1.inodp
#define m_mount u2.mountp
#define m_up	u1.upchan
#define m_down  u2.dnchan

/* alloction flags (m_flags) */

#define	MFREE	0x0	/* Unused - free */
#define	MINUSE	0x1	/* In use - fs mounted on this entry */
#define	MINTER	0x2	/* Mount system call is using this entry */
#define	MRDONLY	0x4	/* File system is mounted read only */

#define	rdonlyfs(mp)	(((mp)->m_flags & MRDONLY) || (server() \
				&& (srmount[u.u_mntindx].sr_flags & MRDONLY)))

/* remoteness flags (m_rflags) */
/* (or'ed with m_flags in srmount, so must use different bits) */

#define MLBIN      0x10
#define MDOTDOT    0x20
#define MFUMOUNT   0x40
#define MLINKDOWN  0x80

/* Flag bits passed to the mount system call */
#define	MS_RDONLY	0x1	/* read only bit */
#define MS_FSS		0x2	/* FSS (4-argument) mount */

#ifdef INKERNEL
extern struct mount mount[];
#endif

/* Server mount structure. Holds:
 *	1. DOT-DOT
 *	2. LBIN
 * connections.
 *
 * Search the table looking for (sysid, rootinode) pair.
 */

 struct srmnt {
	sysid_t sr_sysid;
	char    sr_flags;
	struct inode *sr_rootinode;	/* inode on server's machine */
	int     sr_mntindx;		/* mount index of requestor  */
	int 	sr_refcnt;		/* number of open files      */
	int	sr_dotdot;		/* for lbin mount, so .. works */
	long	sr_bcount;
	int	sr_slpcnt;
} ;
extern struct srmnt srmount[];

/* Return values if crossing mount point */
 
#define DOT_DOT	0x1
#define LBIN	0x2
#define MRDONLY	0x4
