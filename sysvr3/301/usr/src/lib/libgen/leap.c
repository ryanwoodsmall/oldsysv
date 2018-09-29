/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:leap.c	1.2"
/*
	given a year integer, leap returns a one if the year is
	a leap year and a zero otherwise
*/

#include	"libgen.h"

int
leap( year )
int	year;
{
	return  ( ( year % 4 == 0  &&  year % 100 != 0 )  ||
		( year % 400 == 0 ) ) ? 1 : 0 ;
}
