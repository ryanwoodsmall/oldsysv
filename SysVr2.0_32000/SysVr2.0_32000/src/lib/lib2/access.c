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
/*	@(#)access.c	1.1	*/

# include <stand.h>

access(name, mode)
register char *name; {
	register struct iob *io;
	register fdesc;
	int perm;

	if ((fdesc = open(name, 0)) < 0)
		return (-1);

	io = &_iobuf[fdesc - 3];

	if ((io->i_flgs&F_FILE) == 0
	    || io->i_ino.i_mode & ((mode&07) << 6))
		perm = 0;
	else {
		perm = -1;
		errno = EACCES;
	}

	close(fdesc);
	return (perm);
}
