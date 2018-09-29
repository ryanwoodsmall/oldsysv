/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Read.c	1.2"
/*	
	read(2) with error checking
	End-Of-File and incomplete reads are considered errors!
*/

#include	<cmderr.h>
#include	<stdio.h>

Read( fd, buf, count )
int	fd;	/* file descriptor */
char	*buf;
int	count;
{
	register int	countin;

	if( (countin = read( fd, buf, count )) != count ) {
		cmderr( CERROR, "file descriptor %d, asked for %d, got %d",
			fd, count, countin );
	}
}
