/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)lp:date.c	3.2"
/*
 * date(t) -- returns a pointer to the current date and time in ascii
 *	t is a time as returned by time(2)
 */
#include	"lp.h"


#define	DAYSIZE		4
#define	DATESIZE	13

char *
date(t)
time_t t;
{
	char *ctime(), *dp;

	dp = ctime(&t) + DAYSIZE;
	*(dp + DATESIZE -1) = '\0';
	return(dp);
}
