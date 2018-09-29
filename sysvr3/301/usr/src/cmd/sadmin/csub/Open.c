/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Open.c	1.2"
/*	
	open(2) with error checking
*/

#include	<cmderr.h>
#include	<stdio.h>


Open( path, oflag, mode )
char	*path;
int	oflag;
int	mode;
{
	register int	fd;	/* file descriptor */

	if( (fd = open( path, oflag, mode )) == -1 ) {
		cmderr( CERROR, "Cannot open '%s'", path );
		exit( 1 );
	}
	return  fd;
}
