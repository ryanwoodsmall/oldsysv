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
/*	@(#) vsscanf.c: 1.1 10/15/83	(1.1)	*/
#include <stdio.h>
#include <varargs.h>

/*
 *	This routine implements vsscanf (nonportably) until such time
 *	as one is available in the system (if ever).
 */

vsscanf(buf, fmt, ap)
char	*buf;
char	*fmt;
va_list	ap;
{
	FILE	junk;

	junk._flag = _IOREAD;
	junk._file = -1;
	junk._base = junk._ptr = buf;
	junk._cnt = strlen(buf);
	return _doscan(&junk, fmt, ap);
}
