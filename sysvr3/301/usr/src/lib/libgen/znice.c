/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:znice.c	1.2"

/*	nice(2) with error checking
*/

#include	"errmsg.h"

int
znice( severity, incr )
int	 severity;
int	 incr;
{

	int	err_ind;

	if( (err_ind = nice( incr )) == -1 )
	    _errmsg ( "UXznice1", severity,
		  "Cannot add %d to the nice value of the calling process.",
		   incr);

	return err_ind;
}
