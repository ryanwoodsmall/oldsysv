/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zlseek.c	1.2"

/*	lseek(2) with error checking
*/


#include <stdio.h>
#include "errmsg.h"

long
zlseek( severity, fildes, offset, whence)
int	severity;
int	fildes;
long	offset;
int	whence;
{
	int	rc;

	if( (rc = lseek(fildes, offset, whence)) == -1 )
		_errmsg("UXzlseek1", severity,
		       "Cannot set file pointer for fildes '%d', offset '%d', whence '%d'.",
			fildes,offset,whence);


	return  rc;
}
