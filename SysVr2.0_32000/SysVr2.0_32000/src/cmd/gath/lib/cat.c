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
static char Sccsid[] = "@(#)cat.c	1.1";

char	*
cat(buf,s1,s2)
char	*buf,*s1,*s2;
{
	register char	*p,*q;
	p = buf;
	q = s1;
	while (*p++=(*q++));
	p--;
	q = s2;
	while (*p++=(*q++));
	p--;
	*p=0;
	return (p);
};
