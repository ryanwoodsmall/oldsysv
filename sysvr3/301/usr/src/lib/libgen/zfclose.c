/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zfclose.c	1.2"

/*	fclose(3S) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zfclose( severity, stream )
int	severity;
FILE	*stream;
{

	int	err_ind;

	if( (err_ind = fclose( stream )) == EOF )
	    _errmsg( "UXzfclose1", severity,
		   "Cannot close file.");

	return err_ind;
}
