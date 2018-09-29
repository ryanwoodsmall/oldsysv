/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zcalloc.c	1.2"

/*	calloc(3C) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>

char *
zcalloc( severity, nelem, elsize)
int	  severity;
unsigned  nelem, elsize;
{
	char  *p;

	if( ( p = (char *) calloc(nelem, elsize) ) == NULL )
		_errmsg( "UXzcalloc1", severity,
		  "Cannot allocate space for array, %d elements, size %d.",
			nelem, elsize);
	return p;
}
