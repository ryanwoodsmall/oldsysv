/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zplock.c	1.2"

/*	plock(2) with error checking
*/

#include	<sys/lock.h>
#include	"errmsg.h"

int
zplock( severity, op )
int	 severity;
int	 op;
{

	int	err_ind;

	if( (err_ind = plock( op )) == -1 )
	    _errmsg ( "UXzplock1", severity,
		  "Cannot lock, op = %d.",
		   op);

	return err_ind;
}
