/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zopen.c	1.3"

/*	open(2) with error checking
*/

#include	"errmsg.h"
#include	<fcntl.h>
#include	<stdio.h>


zopen(  severity, path, oflag, mode )
int	severity;
char	*path;
int	oflag;
int	mode;
{
	register int	fd;	/* file descriptor */

	if( (fd = open( path, oflag, mode )) == -1 ) {
		_errmsg( "UXzopen1", severity,
			"Cannot open file \"%s\".",
			 path );
	}
	return  fd;
}
