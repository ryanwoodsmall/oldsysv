/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zsetuid.c	1.2"

/*	setuid(2) with error checking
*/

#include	"errmsg.h"

int
zsetuid( severity, uid )
int	 severity;
int	 uid;
{

	int	err_ind;

	if( (err_ind = setuid( uid )) == -1 )
	    _errmsg ( "UXzsetuid1", severity,
		  "Cannot set user id to %d.",
		   uid);

	return err_ind;
}
