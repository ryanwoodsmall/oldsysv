/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zcreat.c	1.2"

/*	creat(2) with error checking
*/

#include <stdio.h>
#include "errmsg.h"

int
zcreat( severity, path, mode )
int  severity;
char *path;
int  mode;
{
	int	rc;

	if( (rc = creat( path, mode ) ) == -1 )
		_errmsg("UXzcreat1", severity,
		       "Cannot create file \"%s\" of mode %d.",
		       path,mode);

	return  rc;
}
