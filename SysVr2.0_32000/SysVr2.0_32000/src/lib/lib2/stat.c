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
/*	@(#)stat.c	1.1	*/

# include <stand.h>
# include <sys/stat.h>

stat (name, buf)
char *name;
struct stat *buf; {
	register fdesc, ret;
	
	fdesc = open (name, 0);
	if (fdesc < 0)
		return (-1);

	ret = fstat (fdesc, buf);
	_iobuf[fdesc-3].i_flgs |= F_FILE;
	close (fdesc);
	return (ret);
}
