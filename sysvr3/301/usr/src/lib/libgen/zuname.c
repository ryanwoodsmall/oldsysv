/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zuname.c	1.2"

/*	uname(2) with error checking
*/

#include	"errmsg.h"
#include	<sys/utsname.h>

int
zuname( severity, name)
int    severity;
struct utsname *name;
{

	int	err_ind;

	if( (err_ind = uname( name )) == -1 )
	    _errmsg("UXzuname1", severity,
		  "Cannot get name of current UNIX system.");

	return err_ind;
}
