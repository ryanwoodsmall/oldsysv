/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zumount.c	1.2"

/*	umount(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zumount( severity, spec )
int	severity;
char	*spec;
{

	int	err_ind;

	if( (err_ind = umount( spec )) == -1 )
	    _errmsg("UXzumount1", severity,
		  "Cannot unmount file system, on device \"%s\".",
		   spec);

	return err_ind;
}
