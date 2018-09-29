/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zwait.c	1.2"

/*	wait(2) with error checking
*/


#include <stdio.h>
#include "errmsg.h"

int
zwait( severity, stat_loc)
int	severity;
int	*stat_loc;
{
	int	rc;

	if( (rc = wait(stat_loc)) == -1 )
		_errmsg("UXzwait1", severity,
		       "Cannot wait for child process to stop/terminate.");

	return  rc;
}
