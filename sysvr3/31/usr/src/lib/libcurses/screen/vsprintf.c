/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/vsprintf.c	1.6"
/*LINTLIBRARY*/
#include "curses_inc.h"
#include <varargs.h>

extern	int	_doprnt();

int
vsprintf(string, format, ap)
char *string, *format;
va_list ap;
{
	register int count;
	FILE siop;

	siop._cnt = MAXINT;
#ifdef _NFILE
	siop._file = _NFILE;
#endif
	siop._flag = _IOWRT;
#ifdef SYSV
	siop._base = siop._ptr = (unsigned char *)string;
#else
	siop._flag |= _IOSTRG;
	siop._ptr = string;
#endif
	count = _doprnt(format, ap, &siop);
	*siop._ptr = '\0'; /* plant terminating null character */
	return (count);
}

int
vfprintf(fp, format, ap)
FILE *fp;
char *format;
va_list ap;
{
	int n;
	char spbuf[512];

	n = vsprintf(spbuf, format, ap);
	(void) fputs(spbuf, fp);
	return (n);
}
