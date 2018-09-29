/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:trim.c	3.2"
/* trim -- trim trailing blanks from character string */

#include	"lp.h"


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
