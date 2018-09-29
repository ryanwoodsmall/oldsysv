/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zioctl.c	1.2"

/*	ioctl(2) with error checking
*/

#include <stdio.h>
#include "errmsg.h"

int
zioctl( severity, fildes, request, arg)
int	severity;
int 	fildes;
int	request;
{
	int	rc;

	if( (rc = ioctl(fildes, request, arg)) == -1 )
		_errmsg("UXzioctl1", severity,
		       "Cannot control device; fildes = %d request = %d arg = %d.",
			fildes, request, arg);

	return  rc;
}
