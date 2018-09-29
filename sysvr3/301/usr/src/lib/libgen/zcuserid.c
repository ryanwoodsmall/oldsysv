/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zcuserid.c	1.2"

/*	cuserid(3S) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

char  *
zcuserid( severity, s )
int	severity;
char	*s;
{

	char	*err_ind;

	if( (err_ind = cuserid( s )) == NULL )
	    _errmsg("UXzcuserid1", severity,
		  "Login name cannot be found.");

	return err_ind;
}
