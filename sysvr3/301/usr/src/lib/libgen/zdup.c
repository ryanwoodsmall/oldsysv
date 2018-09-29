/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zdup.c	1.2"

/*	dup(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zdup( severity, fildes )
int	 severity;
int	 fildes;
{

	int	err_ind;

	if( (err_ind = dup( fildes )) == -1 )
	    _errmsg("UXzdup1", severity,
		  "Cannot duplicate file descriptor %d.",
		   fildes);

	return err_ind;
}
