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
/* @(#)flock.h	1.4 */

/* file segment locking set data type - information passed to system by user */
/* it is also found in fcntl.h */
#ifndef	F_RDLCK
struct	flock	{
	short	l_type;
	short	l_whence;
	long	l_start;
	long	l_len;		/* len = 0 means until end of file */
	int	l_pid;
};
#endif

/* file segment locking types */
#define	F_RDLCK	01	/* Read lock */
#define	F_WRLCK	02	/* Write lock */
#define	F_UNLCK	03	/* Remove lock(s) */

/* file locking structure (connected to file table entry) */
struct	filock	{
	struct	flock set;	/* contains type, start, and length */
	union	{
		int wakeflg;	/* for locks sleeping on this one */
		int blkpid;	/* pid of blocking lock
				 * (for sleeping locks only)
				 */
	}	stat;
	struct	filock *prev;
	struct	filock *next;
};

/* table to associate files with chain of locks */
struct	flino {
	dev_t	fl_dev;
	ino_t	fl_number;
	int	fl_refcnt;	 /* # of procs currently referencing this flino */
	struct	filock *fl_flck; /* pointer to chain of locks for this file */
	struct	flino  *prev;
	struct	flino  *next;
};

/* file and record locking configuration structure */
/* record and file use totals may overflow */
struct flckinfo {
	long recs;	/* number of records configured on system */
	long fils;	/* number of file headers configured on system */
	long reccnt;	/* number of records currently in use */
	long filcnt;	/* number of file headers currently in use */
	long rectot;	/* number of records used since system boot */
	long filtot;	/* number of file headers used since system boot */
};

extern struct flckinfo	flckinfo;
extern struct filock  	flox[];
extern struct flino   	flinotab[];
