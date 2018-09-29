/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zexecvp.c	1.2"

/*	execvp(2) with error checking
*/

#include	"errmsg.h"


zexecvp( severity, file, argv )
int	severity;
char	*file;
char	*argv[];
{
	int  rc;

	if(  (rc = execvp( file, argv )) == -1 )
		_errmsg( "UXzexecvp1", severity,
			"Cannot execute command \"%s\".",
			 file );
	return rc;
}
