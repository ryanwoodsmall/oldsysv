/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/s5readdir.c	1.1"

#include "fslib.h"

/**
 ** s5readdir() - SIMULATE "readdir()" ON ``s5'' TYPE FILESYSTEM
 **/

/*
 * The following is extracted almost verbatim
 * from the SVR3.1 code for "readdir()"; the only
 * change is to use "s5getdents()" instead of
 * "getdents()".
 */
#define	getdents	s5getdents
#define	readdir		s5readdir


struct dirent *
readdir(dirp)
	register DIR		*dirp;	/* stream from opendir() */
	{
	register struct dirent	*dp;	/* -> directory data */
	int saveloc = 0;

	if (dirp->dd_size != 0) {
		dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
		saveloc = dirp->dd_loc;   /* save for possible EOF */
		dirp->dd_loc += dp->d_reclen;
	}
	if (dirp->dd_loc >= dirp->dd_size)
		dirp->dd_loc = dirp->dd_size = 0;

	if (dirp->dd_size == 0 	/* refill buffer */
	  && (dirp->dd_size = getdents(dirp->dd_fd, dirp->dd_buf, 
				    DIRBUF)
	     ) <= 0
	   ) {
		if (dirp->dd_size == 0)	/* This means EOF */
			dirp->dd_loc = saveloc;  /* EOF so save for telldir */
		return 0;	/* error or EOF */
	}

	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	return(dp);
}
#undef	getdents
#undef	readdir
