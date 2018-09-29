/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zfdopen.c	1.2"

/*	fdopen(3S) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>


FILE *
zfdopen( severity, fildes, type )
int	severity;
int	fildes;
char	*type;
{
	register FILE	*fp;	/* file pointer */

	if( (fp = fdopen( fildes, type )) == NULL ) {
		_errmsg( "UXzfdopen1", severity,
			"Cannot fdopen() file descriptor %d for \"%s\".",
			fildes, type );
	}
	return  fp;
}
