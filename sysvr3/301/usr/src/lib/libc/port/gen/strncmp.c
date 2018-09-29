/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strncmp.c	1.7"
/*LINTLIBRARY*/
/*
 * Compare strings (at most n bytes)
 *	returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */
#include <sys/types.h>

int
strncmp(s1, s2, n)
register char *s1, *s2;
register size_t n;
{
	n++;
	if(s1 == s2)
		return(0);
	while(--n > 0 && *s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return((n == 0)? 0: (*s1 - *--s2));
}
