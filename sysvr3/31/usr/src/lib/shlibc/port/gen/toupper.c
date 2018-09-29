/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/toupper.c	1.6"
/*
 * If arg is lower-case, return upper-case, otherwise return arg.
 * International version
 */
#include <ctype.h>

int
toupper(c)
register int c;
{
	if (islower(c))
		c = _toupper(c); 
	return(c);
}
