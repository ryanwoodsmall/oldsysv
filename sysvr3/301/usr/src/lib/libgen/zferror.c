/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zferror.c	1.2"

/*	ferror(3S) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zferror(severity, stream )
int	 severity;
FILE	*stream;
{

	int	err_ind;

	if( (err_ind = ferror( stream )) )
	    _errmsg("UXzferror1", severity,
		  " I/O error has occurred reading/writing ");

	return err_ind;
}
