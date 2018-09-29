/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zfreopen.c	1.3"

/*	freopen(3S) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>


FILE *
zfreopen( severity, path, type, stream )
int	severity;
char	*path;
char	*type;
FILE	*stream;
{
	register FILE	*fp;	/* file pointer */

	if( (fp = freopen( path, type, stream )) == NULL ) {
		_errmsg( "UXzfreopen1", severity,
			"Cannot open file \"%s\".",
			 path );
	}
	return  fp;
}
