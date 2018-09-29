/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zfseek.c	1.2"

/*	fseek(3S) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zfseek( severity, stream, offset, ptrname )
int	 severity;
FILE	*stream;
long	 offset;
int	 ptrname;
{

	int	err_ind;

	if( (err_ind = fseek(stream, offset, ptrname )) < 0 )
	    _errmsg ( "UXzfseek1", severity,
		  "Cannot reposition file pointer, offset %d and ptrname %d.",
		   offset, ptrname );

	return err_ind;
}
