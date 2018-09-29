/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zulimit.c	1.2"

/*	ulimit(2) with error checking
*/

#include	"errmsg.h"

daddr_t
zulimit( severity, cmd, newlimit )
int	 severity;
int	 cmd;
daddr_t	 newlimit;
{
	daddr_t	err_ind;

	if( (err_ind = ulimit(cmd, newlimit )) ==  - 1 )
	    _errmsg("UXzulimit1", severity,
		  "Cannot get or set user limits, cmd %d ,newlimit is %d.",
		   cmd, newlimit);

	return err_ind;
}
