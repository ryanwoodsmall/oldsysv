/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:num.c	1.2"
/*	
	true if string is numeric characters
*/

#include	<ctype.h>
#include	"libgen.h"


int
num( string )
register char *string;
{
	if( !string  ||  !*string )
		return  0;	/* null pointer or null string */
	while( *string )
		if( !isdigit( *(string++) ) )
			return  0;
	return  1;
}
