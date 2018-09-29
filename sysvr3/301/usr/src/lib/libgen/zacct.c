/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zacct.c	1.2"

/*	acct(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zacct( severity, path )
int     severity;
char	*path;
{

	int	err_ind;

	if( (err_ind = acct( path )) == -1 )
	    _errmsg("UXzacct1", severity,
		  "Cannot enable/disable sys. processing routine path \"%s\".",
		   path);

	return err_ind;
}
