/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zchmod.c	1.2"

/*	chmod(2) with error checking
*/

#include	"errmsg.h"

int
zchmod( severity, path, mode )
int	 severity;
char	*path;
int	 mode;
{

	int	err_ind;

	if( (err_ind = chmod(path, mode )) == -1 )
	    _errmsg ( "UXzchmod1", severity,
		  "Cannot change the mode of file \"%s\" to %d.",
		   path, mode);

	return err_ind;
}
