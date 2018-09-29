/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/nstrcat.c	1.1"

#include	<varargs.h>

/*
 * useful for creating strings for error messages
 */
char *
nstrcat(va_alist)
va_dcl
{
	register char	*p, *q;
	static char	buf[128];
	va_list	ap;

	va_start(ap);
	for (q = buf; q < &buf[sizeof(buf) - 1] && (p = va_arg(ap, char *)); q += strlen(q))
		strncpy(q, p, &buf[sizeof(buf) - 1] - q);
	va_end(ap);
	return buf;
}
