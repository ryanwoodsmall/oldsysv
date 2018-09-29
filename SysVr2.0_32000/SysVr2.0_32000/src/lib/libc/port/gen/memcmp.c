/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)memcmp.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Compare n bytes:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */
int
memcmp(s1, s2, n)
register char *s1, *s2;
register int n;
{
	int diff;

	if (s1 != s2)
		while (--n >= 0)
			if (diff = *s1++ - *s2++)
				return (diff);
	return (0);
}
