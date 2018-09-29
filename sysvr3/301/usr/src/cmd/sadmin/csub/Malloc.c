/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/Malloc.c	1.2"
/*	
	malloc(3C) with error checking.
*/

#include <stdio.h>
#include "cmderr.h"


char *
Malloc(n)
unsigned	n;
{
	char	*p;

	if ((p = (char *)malloc(n)) == NULL) 
		cmderr(CERROR, CERRNO);

	return  p;
}
