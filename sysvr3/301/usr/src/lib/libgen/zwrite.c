/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zwrite.c	1.2"

/*	write(2) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>


zwrite( severity, fd, buf, count )
int	severity;
int	fd;	/* file descriptor */
char	*buf;
unsigned int	count;
{
	register int	countout;

	if( (countout = write( fd, buf, count )) != count ) {
		_errmsg( "UXzwrite1", severity,
			"File descriptor %d, tried %d, wrote %d.",
			fd, count, countout );
	}
}
