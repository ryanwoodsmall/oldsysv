/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:sweekday.c	1.2"
/* sweekday -- return string name of day of the week
**
** This routine returns the name of the day of the week that
** corresponds to an absolute date.  Day 1 is Saturday.  The
** static array therefore begins with Friday at [0].
*/

#include	"libgen.h"

/* Table of names of days.  Day 1 is a Saturday. */

static char * dayname[] = {
	"Friday", "Saturday", "Sunday", "Monday",
	"Tuesday", "Wednesday", "Thursday"
};

char *
sweekday(day)
long day;				/* absolute date */
{
    return( dayname[day%7] );
}
