/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/conf.h	10.4"
/*
 * Declaration of block device switch. Each entry (row) is
 * the only link between the main unix code and the driver.
 * The initialization of the device switches is in the file conf.c.
 */
struct bdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_print)();
};
extern struct bdevsw bdevsw[];

/*
 * Character device switch.
 */
struct cdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	struct tty *d_ttys;
	struct streamtab *d_str;
};
extern struct cdevsw cdevsw[];

#define	FMNAMESZ	8

struct fmodsw {
	char	f_name[FMNAMESZ+1];
	struct  streamtab *f_str;
};
extern struct fmodsw fmodsw[];

extern int	bdevcnt;
extern int	cdevcnt;
extern int	fmodcnt;

/*
 * Line discipline switch.
 */
struct linesw {
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
};
extern struct linesw linesw[];

extern int	linecnt;
/*
 * Terminal switch
 */
struct termsw {
	int	(*t_input)();
	int	(*t_output)();
	int	(*t_ioctl)();
};
extern struct termsw termsw[];

extern int	termcnt;

/*file system switch structure */
struct fstypsw {
/* 0*/	int		(*fs_init)();
/* 1*/	int		(*fs_iput)();
/* 2*/	struct inode	*(*fs_iread)();
/* 3*/	int		(*fs_filler)();
/* 4*/	int		(*fs_iupdat)();
/* 5*/	int		(*fs_readi)();
/* 6*/	int		(*fs_writei)();
/* 7*/	int		(*fs_itrunc)();
/* 8*/	int		(*fs_statf)();
/* 9*/	int		(*fs_namei)();
/*10*/	int		(*fs_mount)();
/*11*/	int		(*fs_umount)();
/*12*/	struct inode	*(*fs_getinode)();
/*13*/	int		(*fs_openi)();		/* open inode */
/*14*/	int		(*fs_closei)();		/* close inode */
/*15*/	int		(*fs_update)();		/* update */
/*16*/	int		(*fs_statfs)();		/* statfs and ustat */
/*17*/	int		(*fs_access)();
/*18*/	int		(*fs_getdents)();
/*19*/	int		(*fs_allocmap)();	/* Let the fs decide if */
						/* if can build a map so */
						/* this fs can be used for */
						/* paging */
/*20*/	int		*(*fs_freemap)();	/* free block list */
/*21*/	int		(*fs_readmap)();	/* read a page from the fs */
						/* using the block list */
/*22*/	int		(*fs_setattr)();	/* set attributes */
/*23*/	int		(*fs_notify)();		/* notify fs of action */
/*24*/	int		(*fs_fcntl)();		/* fcntl */
/*25*/	int		(*fs_fsinfo)();		/* additional info */
/*26*/	int		(*fs_ioctl)();		/* ioctl */
/*27*/	int		(*fs_fill[5])();
};
extern struct fstypsw fstypsw[];
extern short nfstyp;


/* FS specific data */
struct fsinfo {
	long		fs_flags;	/* flags - see below */
	struct mount	*fs_pipe;	/* The mount point to be used */
					/* as the pipe device for */
					/* this fstyp */
	char		*fs_name; 	/* Pointer to fstyp name */
					/* See above */
	long		fs_notify;	/* Flags for fs_notify */
					/* e.g., NO_CHDIR, NO_CHROOT */
					/* see nami.h */
};
extern struct fsinfo fsinfo[];
