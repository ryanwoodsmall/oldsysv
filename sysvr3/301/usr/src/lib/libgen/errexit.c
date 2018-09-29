/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:errexit.c	1.1"

#include  "errmsg.h"

/*	This routine sets the exit(2) value for exit actions.
	It returns the previous value.
*/

int
errexit( e )
int	e;
{
	int	oe;

	oe = Err.exit;
	Err.exit = e;
	return  oe;
}
