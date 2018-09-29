/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:day.c	1.3"
/*
	day - given julian date and year returns day of week
	based on the fact that January 1st, 1800, was a Wednesday.
*/

#include	<stdio.h>
#include	"libgen.h"

static char	*dayofwk[] = { "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", "Mon" };

#define	leap(y)	( y%4 == 0  &&  y%100 != 0  ||  y%400 == 0 )

char *
day(date, y)
int		date;	/* Julian day of year, 1 through 366 */
register int	y;	/* year */
{
	register long	jdate;
	register int	year;

	if ( date > 366 || date <= 0 || (date == 366 && ! leap( y )))
		return 0;
	if ( y < 1800 )
		return   0;
	jdate = date;
	for( year = 1800 ; year < y ; year++ ) {
		jdate = jdate + ( leap( year ) ? 366 : 365 );
	}
	return dayofwk[ jdate % 7];
}
