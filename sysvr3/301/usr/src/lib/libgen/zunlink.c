/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zunlink.c	1.2"

/*	unlink(2) with error checking
*/


#include <stdio.h>
#include "errmsg.h"

int
zunlink( severity, path)
int	severity;
char	*path;
{
	int	rc;

	if( (rc = unlink(path)) == -1 )
		_errmsg("UXzunlink1", severity,
		       "Cannot remove directory entry \"%s\".",
			path);

	return  rc;
}
