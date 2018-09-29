/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Execvp.c	1.2"
/*
	Run execvp(2) with error checking.
*/

#include	<cmderr.h>


Execvp( file, argv )
char	*file;
char	*argv[];
{
	execvp( file, argv );
	cmderr( CERROR, "Cannot find command '%s'", file );
	exit( 1 );	/* in case cmderr() fails to exit */
}
