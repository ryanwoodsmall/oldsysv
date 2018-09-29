/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)liberr:zrealloc.c	1.2"

/*	realloc(3C) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>

char *
zrealloc( severity, ptr, size)
int	  severity;
char	  *ptr;
unsigned  size;
{
	 char *p;

	if( ( p = (char *)realloc(ptr, size) ) == NULL )
		_errmsg("UXzrealloc1", severity,
		       "Cannot reallocate %d bytes.",
			size);
	return p;
}
