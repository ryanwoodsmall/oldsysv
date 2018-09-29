/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zclose.c	1.2"

/*	close(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zclose( severity, fildes )
int	 severity;
int	 fildes;
{

	int	err_ind;

	if( (err_ind = close( fildes )) == -1 )
	    _errmsg("UXzclose1", severity,
		  "Cannot close file descriptor %d.",
		   fildes);

	return err_ind;
}
