/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zmknod.c	1.2"

/*	mknod(2) with error checking
*/

#include	<stdio.h>
#include	"errmsg.h"

int
zmknod( severity, path, mode, dev )
int	severity;
char	*path;
int	mode;
int	dev;
{

	int	err_ind;

	if( (err_ind = mknod(path, mode, dev )) == -1 )
	    _errmsg("UXzmknod1", severity,
		  "Cannot create file \"%s\" of mode %d on device %d.",
		   path, mode, dev);

	return err_ind;
}
