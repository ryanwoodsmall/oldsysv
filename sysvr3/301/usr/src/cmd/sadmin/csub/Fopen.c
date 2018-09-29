/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Fopen.c	1.2"
/*
	fopen(3S) with error checking
*/

#include	<cmderr.h>
#include	<stdio.h>


FILE *
Fopen( path, type )
char	*path;
char	*type;
{
	register FILE	*fp;	/* file pointer */

	if( (fp = fopen( path, type )) == NULL ) {
		cmderr( CERROR, "Cannot open '%s'", path );
	}
	return  fp;
}
