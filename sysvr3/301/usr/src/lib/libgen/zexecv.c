/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zexecv.c	1.2"

/*	execv(2) with error checking
*/

#include	"errmsg.h"


zexecv( severity, file, argv )
int	severity;
char	*file;
char	*argv[];
{
	int	rc;

	if( (rc = execv( file, argv )) == -1 )
		_errmsg( "UXzexecv1", severity,
			"Cannot execute command \"%s\".",
			 file );
	return rc;
}
