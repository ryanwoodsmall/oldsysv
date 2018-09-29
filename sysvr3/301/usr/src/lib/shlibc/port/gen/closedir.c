/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/closedir.c	1.4"
/*
	closedir -- C library extension routine

*/

#include	<sys/types.h>
#include	<dirent.h>


extern void	free();
extern int	close();

int
closedir( dirp )
register DIR	*dirp;		/* stream from opendir() */
{
	free( dirp->dd_buf );
	free( (char *)dirp );
	return(close( dirp->dd_fd ));
}
