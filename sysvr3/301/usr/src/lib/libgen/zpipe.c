/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zpipe.c	1.2"

/*	pipe(2) with error checking
*/

#include	"errmsg.h"

int
zpipe( severity, fildes )
int	severity;
int	 fildes[2];
{

	int	err_ind;

	if( (err_ind = pipe( fildes )) == -1 )
	    _errmsg("UXzpipe1", severity,
		  "Cannot pipe");

	return err_ind;
}
