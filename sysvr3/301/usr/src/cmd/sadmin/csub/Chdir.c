/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Chdir.c	1.2"

#include	<cmderr.h>


void
Chdir( path )
char	*path;
{
	if( !chdir( path ) )
		return;
	cmderr( CERROR, "Cannot change to directory '%s'.", path );
	exit( 1 );	/* in case cmderr does not exit */
}
