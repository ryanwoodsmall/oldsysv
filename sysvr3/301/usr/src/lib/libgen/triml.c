/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:triml.c	1.2"
/*
	Trim leading characters from a string.
	Returns pointer to first character in string not in tc.
*/

#include	<string.h>
#include	"libgen.h"


char *
triml( string, tc )
char	*string;
char	*tc;	/* characters to trim */
{
	int	l;

	l = strspn( string, tc );
	return  string + l;
}
