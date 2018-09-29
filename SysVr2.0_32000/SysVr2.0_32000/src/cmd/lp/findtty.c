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
/* findtty(user) -- find first tty that user is logged in to.
	returns: /dev/tty?? if user is logged in
		 NULL, if not
*/

#include	"lp.h"
#include	"lpsched.h"

SCCSID("@(#)findtty.c	3.1")

char *
findtty(user)
char *user;
{
	struct utmp utmp;
	static char tty[15];
	FILE *u;

	if((u = fopen(UTMP, "r")) == NULL)
		return(NULL);
	while(fread((char *) &utmp, sizeof(struct utmp), 1, u) == 1)
		if(strcmp(utmp.ut_name, user) == 0) {
			sprintf(tty, "/dev/%s", utmp.ut_line);
			fclose(u);
			return(tty);
		}
	fclose(u);
	return(NULL);
}
