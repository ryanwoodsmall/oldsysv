/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Write.c	1.2"
/*	
	write(2) with error checking
*/

#include	<cmderr.h>
#include	<stdio.h>


Write( fd, buf, count )
int	fd;	/* file descriptor */
char	*buf;
int	count;
{
	register int	countout;

	if( (countout = write( fd, buf, count )) != count ) {
		cmderr( CERROR, "file descriptor %d, tried %d, wrote %d",
			fd, count, countout );
	}
}
