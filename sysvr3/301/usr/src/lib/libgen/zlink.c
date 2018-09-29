/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zlink.c	1.2"

/*	link(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zlink( severity, path1, path2 )
int	severity;
char	*path1;
char	*path2;
{

	int	err_ind;

	if( (err_ind = link(path1, path2 )) == -1 )
	    _errmsg("UXzlink1", severity,
		  "Cannot create link for \"%s\" to file \"%s\".",
		   path2 , path1);

	return err_ind;
}
