/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/putw.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * The intent here is to provide a means to make the order of
 * bytes in an io-stream correspond to the order of the bytes
 * in the memory while doing the io a `word' at a time.
 */
#include "shlib.h"
#include <stdio.h>

int
putw(w, stream)
int w;
register FILE *stream;
{
	register char *s = (char *)&w;
	register int i = sizeof(int);

	while (--i >= 0)
		(void) putc(*s++, stream);
	return (ferror(stream));
}
