/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:strnlen.c	1.2"
/*LINTLIBRARY*/
/*
 * Returns the number of
 * non-NULL bytes in string argument
 * but no more than n.
 */

#include	"libgen.h"


int
strnlen(s, n)
char	*s;
int	n;
{
	register char	*sp = s;
	register char	*send = sp + n;

	for( ;  sp < send  &&  *sp != '\0';  sp++ )
		;
	return (sp - s);
}
