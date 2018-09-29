/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zchdir.c	1.2"

/*	chdir(2) with error checking
*/

#include	"errmsg.h"


int
zchdir( severity, path )
int     severity;
char	*path;
{
	int	rc;

	if( (rc = chdir( path )) == -1 )
	_errmsg( "UXzchdir1", severity,
		"Cannot change to directory \"%s\".", path );
	return rc;

}
