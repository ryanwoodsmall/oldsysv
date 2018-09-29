/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zexecve.c	1.2"

/*	execve(2) with error checking
*/

#include	"errmsg.h"


zexecve( severity, file, argv, envp )
int	severity;
char	*file;
char	*argv[];
char	*envp[];
{
	int	rc;

	if( (rc = execve( file, argv, envp )) == -1 )
		_errmsg( "UXzexecve1", severity,
			"Cannot execute command \"%s\".",
			 file );
	return rc;
}
