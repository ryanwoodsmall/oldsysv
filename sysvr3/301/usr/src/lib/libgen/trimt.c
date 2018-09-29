/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:trimt.c	1.2"
/*
	Trim trailing characters from a string.
	Returns pointer to last character in string not in tc.
*/

#include	<string.h>
#include	"libgen.h"


char *
trimt( string, tc )
char	*string;
char	*tc;	/* characters to trim */
{
	char	*p;

	p = string + strlen( string );  
	while( p != string )
		if( !strchr( tc, *--p ) )
			return  ++p;
		
	return  p;
}
