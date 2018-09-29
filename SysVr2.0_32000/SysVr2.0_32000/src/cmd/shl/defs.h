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
/*	@(#)defs.h	1.3	*/
/*	@(#)defs.h	1.2	*/
#include	<sys/types.h>
#include	<sys/tty.h>
#include	<sys/errno.h>
#include	<signal.h>
#include	<sys/termio.h>
#include	<sys/sxt.h>
#include	<fcntl.h>
#include	<stdio.h>

#define		SYSERROR	-1
#define		MAX_LAYERS	MAXPCHAN
#define		NAMSIZ		8
#define		set_dev(x)	sprintf(&cntlf[9], "%03d", conv(x))
#define		prompt()	printf(">>> ");

struct layer
{
	char name[NAMSIZ + 1];
	int p_grp;
};

extern struct layer 	*layers[];
extern int		 		chan;
extern int 				max_index;
extern int				fildes[2];

extern char 			cntlf[];
extern int  			cntl_chan_fd;

extern int				errno;
extern int				sys_nerr;
extern char				*sys_errlist[];

extern struct utmp		*u_entry;
extern struct termio	ttysave;

extern int				uid;
extern int				gid;

char 					*strcpy();
int						hangup();
int						child_death();


