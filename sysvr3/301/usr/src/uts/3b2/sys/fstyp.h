/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/fstyp.h	10.10"
#define NULL_FS	0		/* Null FS type - Invalid */

#define FSTYPSZ		16	/* max size of fs identifier */

/*
 * Opcodes for the sysfs() system call.
 */
#define GETFSIND	1	/* translate fs identifier to fstype index */
#define GETFSTYP	2	/* translate fstype index to fs identifier */
#define GETNFSTYP	3	/* return the number of fstypes */

/* Bit settings for fs_flags */
#define FS_NOICACHE	0x1	/* Retain old inodes in in-core cache. */
				/* Should an iput be done on last */
				/* reference?  If this flag is set, an */
				/* iput() is done.  System V fs should have */
				/* this clear */

#define	FSBSIZE(ip)	((ip)->i_mntdev)->m_bsize

/* Passed to fs_getinode to indicate intended use of inode */
#define FSG_PIPE	1	/* Pipe */
#define FSG_CLONE	2	/* Use for clone device */

extern	short	pipefstyp;

#ifdef FSPTR

#define FS_IPUT(ip) (*(ip)->i_fstypp->fs_iput)(ip)
#define FS_IREAD(ip) (*(ip)->i_fstypp->fs_iread)(ip)
#define FS_IUPDAT(ip, tm1, tm2) (*(ip)->i_fstypp->fs_iupdat)(ip, tm1, tm2)
#define FS_READI(ip) (*(ip)->i_fstypp->fs_readi)(ip)
#define FS_WRITEI(ip) (*(ip)->i_fstypp->fs_writei)(ip)
#define FS_ITRUNC(ip) (*(ip)->i_fstypp->fs_itrunc)(ip)
#define FS_STATF(ip, arg) (*(ip)->i_fstypp->fs_statf)(ip, arg)
#define FS_NAMEI(ip, p, arg) (*(ip)->i_fstypp->fs_namei)(p, arg)
#define FS_OPENI(ip, mode) (*(ip)->i_fstypp->fs_openi)(ip, mode)
#define FS_CLOSEI(ip, f, c, o) (*(ip)->i_fstypp->fs_closei)(ip, f, c, o)
#define FS_ACCESS(ip, mode) (*(ip)->i_fstypp->fs_access)(ip, mode)
#define FS_GETDENTS(ip, bp, bsz) (*(ip)->i_fstypp->fs_getdents)(ip, bp, bsz)
#define FS_ALLOCMAP(ip) (*(ip)->i_fstypp->fs_allocmap)(ip)
#define FS_FREEMAP(ip) (*(ip)->i_fstypp->fs_freemap)(ip)
#define FS_READMAP(ip, off, sz, va, sf) (*(ip)->i_fstypp->fs_readmap)(ip, off, sz, va, sf)
#define FS_SETATTR(ip, mode) (*(ip)->i_fstypp->fs_setattr)(ip, mode)
#define FS_NOTIFY(ip, arg) (*(ip)->i_fstypp->fs_notify)(ip, arg)
#define FS_FCNTL(ip, cmd, arg, flag, offset) (*(ip)->i_fstypp->fs_fcntl)(ip, cmd, arg, flag, offset)
#define FS_IOCTL(ip, cmd, arg, flag) (*(ip)->i_fstypp->fs_ioctl)(ip, cmd, arg, flag)

#else

#define FS_IPUT(ip) (*fstypsw[(ip)->i_fstyp].fs_iput)(ip)
#define FS_IREAD(ip) (*fstypsw[(ip)->i_fstyp].fs_iread)(ip)
#define FS_IUPDAT(ip, tm1, tm2) (*fstypsw[(ip)->i_fstyp].fs_iupdat)(ip, tm1, tm2)
#define FS_READI(ip) (*fstypsw[(ip)->i_fstyp].fs_readi)(ip)
#define FS_WRITEI(ip) (*fstypsw[(ip)->i_fstyp].fs_writei)(ip)
#define FS_ITRUNC(ip) (*fstypsw[(ip)->i_fstyp].fs_itrunc)(ip)
#define FS_STATF(ip, arg) (*fstypsw[(ip)->i_fstyp].fs_statf)(ip, arg)
#define FS_NAMEI(ip, p, arg) (*fstypsw[(ip)->i_fstyp].fs_namei)(p, arg)
#define FS_OPENI(ip, mode) (*fstypsw[(ip)->i_fstyp].fs_openi)(ip, mode)
#define FS_CLOSEI(ip, f, c, o) (*fstypsw[(ip)->i_fstyp].fs_closei)(ip, f, c, o)
#define FS_ACCESS(ip, mode) (*fstypsw[(ip)->i_fstyp].fs_access)(ip, mode)
#define FS_GETDENTS(ip, bp, bsz) (*fstypsw[(ip)->i_fstyp].fs_getdents)(ip, bp, bsz)
#define FS_ALLOCMAP(ip) (*fstypsw[(ip)->i_fstyp].fs_allocmap)(ip)
#define FS_FREEMAP(ip) (*fstypsw[(ip)->i_fstyp].fs_freemap)(ip)
#define FS_READMAP(ip, off, sz, va, sf) (*fstypsw[(ip)->i_fstyp].fs_readmap)(ip, off, sz, va, sf)
#define FS_SETATTR(ip, mode) (*fstypsw[(ip)->i_fstyp].fs_setattr)(ip, mode)
#define FS_NOTIFY(ip, arg) (*fstypsw[(ip)->i_fstyp].fs_notify)(ip, arg)
#define FS_FCNTL(ip, cmd, arg, flag, offset) (*fstypsw[(ip)->i_fstyp].fs_fcntl)(ip, cmd, arg, flag, offset)
#define FS_IOCTL(ip, cmd, arg, flag) (*fstypsw[(ip)->i_fstyp].fs_ioctl)(ip, cmd, arg, flag)

#endif
