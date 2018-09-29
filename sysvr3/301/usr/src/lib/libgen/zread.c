/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zread.c	1.2"

/*	read(2) with error checking
	End-Of-File and incomplete reads are considered errors!
*/

#include	"errmsg.h"
#include	<stdio.h>

zread(  severity, fd, buf, count )
int	severity;
int	fd;	/* file descriptor */
char	*buf;
unsigned int	count;
{
	register int	countin;

	if( (countin = read( fd, buf, count )) != count ) {
		_errmsg( "UXzread1", severity,
			"File descriptor %d, asked for %d, got %d.",
			 fd, count, countin );
	}
}
