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
/* trim -- trim trailing blanks from character string */

#include	"lp.h"

SCCSID("@(#)trim.c	3.1")

char *
trim(s)
char *s;
{
	int len;

	if((len = strlen(s)) != 0) {
		while(s[--len] == ' ')
			;
		s[++len] = '\0';
	}

	return(s);
}
