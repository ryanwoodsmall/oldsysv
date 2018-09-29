/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zsetgid.c	1.2"

/*	setgid(2) with error checking
*/

#include	"errmsg.h"

int
zsetgid( severity, gid )
int	 severity;
int	 gid;
{

	int	err_ind;

	if( (err_ind = setgid( gid )) == -1 )
	    _errmsg ( "UXzsetgid1", severity,
		  "Cannot set group id to %d.",
		   gid);

	return err_ind;
}
