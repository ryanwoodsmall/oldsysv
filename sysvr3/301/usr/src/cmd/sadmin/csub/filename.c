/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/filename.c	1.2"
/*	
	If the global character pointer is non-null, print the name it
	points to on the stderr.
	Used for error messages usually.
*/


#include	<stdio.h>
#include	<stddef.h>


filename( )
{
	if( file_name )
		fprintf( stderr, "%s:  ", file_name );
}
