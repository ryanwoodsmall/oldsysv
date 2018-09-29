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
static char Sccsid[]="@(#)fdfopen.c	3.2";
/*
	Interfaces with /lib/libS.a
	First arg is file descriptor, second is read/write mode (0/1).
	Returns file pointer on success,
	NULL on failure (no file structures available).
*/

# include	"stdio.h"
# include	"sys/types.h"
# include	"macros.h"

FILE *
fdfopen(fd, mode)
register int fd, mode;
{
	if (fstat(fd, &Statbuf) < 0)
		return(NULL);
	if (mode)
		return(fdopen(fd,"w"));
	else
		return(fdopen(fd,"r"));
}
