/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zchroot.c	1.2"

/*	chroot(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zchroot(  severity, path )
int	severity;
char	*path;
{

	int	err_ind;

	if( (err_ind = chroot( path )) == -1 )
	    _errmsg("UXzchroot1", severity,
		  "Cannot change root directory to \"%s\".",
		   path);

	return err_ind;
}
