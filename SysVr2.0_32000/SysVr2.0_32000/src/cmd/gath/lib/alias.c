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
static char Sccsid[] = "@(#)alias.c	1.1";

alias(s,t)
char *s,*t;
{
	register char *p,*px,*q;
	char *scan();
	q=s;
	for (p=s;*p;p++)
	{
		if (*p=='/') q=p;
	};
	if ((p=scan("hasp",q)) || (p=scan("uvac",q)) || (p=scan("nlsc",q)))
	{
		px=p+4;
		if (*px>='0' && *px<='9') px++;
		for (q=t;p<px;) *q++=(*p++);
		*q=0;
		return (0);
	};
	return (-1);
};
