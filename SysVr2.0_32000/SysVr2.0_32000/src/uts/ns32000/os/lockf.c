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
/* @(#)lockf.c	1.1 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

lockf(fildes, function, size)
long size;
int fildes, function;
{
	struct flock l;
	int rv;

	l.l_whence = 1;
	if (size < 0) {
		l.l_start = size;
		l.l_len = -size;
	} else {
		l.l_start = 0L;
		l.l_len = size;
	}
	switch (function) {
	case F_ULOCK:
		l.l_type = F_UNLCK;
		return (fcntl(fildes, F_SETLK, &l));
	case F_LOCK:
		l.l_type = F_WRLCK;
		return (fcntl(fildes, F_SETLKW, &l));
	case F_TLOCK:
		l.l_type = F_WRLCK;
		return (fcntl(fildes, F_SETLK, &l));
	case F_TEST:
		l.l_type = F_WRLCK;
		if ((rv = fcntl(fildes, F_GETLK, &l)) < 0)
			return (rv);
		if (l.l_type == F_UNLCK)
			return (0);
		else {
			errno = EACCES;
			return (-1);
		}
	default:
		errno = EINVAL;
		return (-1);
	}
}
