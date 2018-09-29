/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:dirname.c	1.2"

/*
	Return pointer to the last element of a pathname.
*/

#include	<string.h>
#include	"libgen.h"


char *
dirname( s )
char	*s;
{
	register char	*p;

	if( !s  ||  !*s )			/* zero or empty argument */
		return  ".";

	p = s + strlen( s );
	while( p != s  &&  *--p == '/' )	/* trim trailing /s */
		;
	
	while( p != s )
		if( *--p == '/' ) {
			*p = '\0';
			return  s;
		}
	
	return  ".";
}
