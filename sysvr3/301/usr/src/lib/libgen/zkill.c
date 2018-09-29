/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zkill.c	1.2"

/*	kill(2) with error checking
*/


#include <stdio.h>
#include "errmsg.h"

int
zkill( severity, pid, sig)
int 	severity;
int 	pid;
int	sig;
{
	int	rc;

	if( (rc = kill(pid, sig)) == -1 )
		_errmsg("UXzkill1", severity,
		       "Cannot send signal %d to process %d.",
			sig,pid);

	return  rc;
}
