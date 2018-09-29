/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:waitpid.c	1.2"
/*	Issue a wait(2) until the expected process id (pid) comes back.
*/

#include	<errno.h>
#include	"libgen.h"


int
waitpid( pid, statusp )
int	pid;
int	*statusp;
{
	int	rc;

	while( (rc = wait( statusp )) != pid  &&  rc != -1 )
		;
	return  rc;
}
