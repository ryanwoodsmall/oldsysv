/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:to_date.c	1.2"
/*
** 1.	Years between 1 and 1752 follow the Julian calendar:
**	every fourth year has 366 days.
**
** 2.	1752 was the changeover year from Julian to Gregorian
**	calendar.  Anomalous behavior occurs in September,
**	when the succession of days is 1, 2, 14, 15, ....
**	Thus September has 19 days, and the year has 355
**	(it was a leap year).
**
** 3.	After 1752, the Gregorian calendar dictates that a
**	leap year occurs every fourth year that is not
**	divisible by 100, unless it is also divisible by 400.
**
**
** In this counting scheme, day 1 is Jan. 1, 1.  9/2/1752 and
** 9/14/1752 are consecutive days ordinally.
*/

/* These tables are useful for the conversion process */

/* Number of days up to first of each month for various types of year. */

/* Normal non-leap year */

static int norm_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 59,				/* Jan, Feb, Mar */
    90, 120, 151,			/* Apr, May, Jun */
    181, 212, 243,			/* Jul, Aug, Sep */
    273, 304, 334,			/* Oct, Nov, Dec */
    365					/* end of year */
};

/* Normal leap year */

static int leap_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 60,				/* Jan, Feb, Mar */
    91, 121, 152,			/* Apr, May, Jun */
    182, 213, 244,			/* Jul, Aug, Sep */
    274, 305, 335,			/* Oct, Nov, Dec */
    366					/* end of year */
};

/* That odd-ball year, 1752.  Note September! */

static int y1752_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 60,				/* Jan, Feb, Mar */
    91, 121, 152,			/* Apr, May, Jun */
    182, 213, 244,			/* Jul, Aug, Sep */
    263, 294, 324,			/* Oct, Nov, Dec */
    355					/* end of year */
};

#include	"libgen.h"

/* to_date -- convert ordinal day to month, day, year
**
** This routine does the inverse of to_day.  It converts
** the ordinal day to month/day/year, taking into account
** all of the anomalies stated above.
*/

void
to_date(day, pmonth, pday, pyear)
long day;				/* input ordinal day */
int * pmonth;				/* output month */
int * pday;				/* output day */
int * pyear;				/* output year */
{
    int year = 0;			/* local year */
    int month;				/* local month */
    int * upto = norm_upto;		/* pointer to array of days up to each
					** month; assume normal year
					*/

    /* These three routines return a 0-based year.  Beware! */
    void doleap();
    void do100();
    void do400();

    /* Converting in this direction is immensely more difficult
    ** than the preceding routine.  We have to keep pecking away
    ** until we figure out which year the day falls in, then
    ** find the month and day.  All of this works more easily if
    ** we make the day 0-based, rather than 1-based, so we start
    ** by subtracting 1.
    */

    day--;

    /* Years up to 1752 */

    if (day < ((1752-1)*365L + (1752-1)/4))
    {
	doleap(&day, &year, 3);		/* simple Julian procedure; first
					** leap year is 3 hence
					*/
	if( year%4 == 3 )		/* leap year (zero-based) */
	    upto = leap_upto;
    }

    /* Year 1752 only (remember the correction before end of year) */

    else if ( day < ((1753-1)*365L + (1753-1)/4 - 11) )
    {
	day -= (1752-1)*365L + (1752-1)/4;
	year = 1752 - 1;		/* years are zero-based */
	upto = y1752_upto;
    }

    /* Years 1753 to 1800 */

    else if ( day < ((1800-1)*365L + (1800-1)/4 - 11))
    {
	/* Treat this like pre-1752, since no funny leap year until 1800.
	** Therefore, cheat by adding back missing 11 days.
	*/

	day += 11;
	doleap(&day, &year, 1756-1753);	/* first leap year is 1756 */
	if ( year%4 == 3 )		/* use leap year table */
	    upto = leap_upto;
    }

    /* Years 1800 to 2000.  Deduct century year correction, leave 1752
    ** correction.
    */

    else if ( day < ((2000-1)*365L + (2000-1)/4 - 2 - 11) )
    {
	/* Change basis to be first day of 1800 */

	day -= (1800-1)*365L + (1800-1)/4 - 11;
	year = 1799;			/* zero-based, remember */
	do100(&day, &year);		/* handle centuries */
	if (year%4 == 3 && year%100 != 99)
	    upto = leap_upto;		/* year is a leap year */
    }

    /* Years 2000 and after.  Base things from first day of year 2000. */

    else
    {
	day -= (2000-1)*365L + (2000-1)/4 - 2 - 11;
	year = 1999;
	do400(&day, &year);		/* get accurate year, day */
	/* check for leap year */

	if (year%4 == 3 && (year%100 != 99 || year%400 == 399))
	    upto = leap_upto;
    }
/* At this point the hard stuff is done.  "year" is the correct year
** minus 1, "day" is the day of the year within "year" (zero-based:
** 1/1 is zero).  Get month, correct day and year, and return.
*/

    for (month = 2; day >= upto[month] ; month++)
	;
    
    month--;				/* went one too far */

    /* Fudge for 1752 anomaly */

    *pyear = year+1;
    *pmonth = month;
    *pday = day - upto[month] + 1;
    if (year == 1751 && month == 9 && *pday > 2)
	*pday += 11;
    return;
}
/* doleap -- handle simple Julian leap year figuring
**
** This routine takes a day and figures out the relative year
** and day of year, assuming a simple Julian calendar (leap
** year every four years).  The zero-th relative year must be
** a leap year for this to work correctly.
*/

static void
doleap(pdays, pyear, firstleap)
long * pdays;				/* ordinal day */
int * pyear;				/* relative year */
int firstleap;				/* number of years to first leapyear */
{
    int nquad;				/* number of leap intervals */
    int nrem;				/* left-over days from such hunks */
    int nyear;				/* number of years within leap
					** interval
					*/
    if (*pdays < firstleap*365)
    {
	*pyear += *pdays / 365;		/* simple non-leap calculation */
	*pdays %= 365;
	return;
    }
    else
    {
	*pyear += firstleap;		/* discard first non-leap interval */
	*pdays -= 365*firstleap;
    }

    nquad = *pdays / (365*4+1);		/* number of 4-year hunks */
    nrem = *pdays % (365*4+1);		/* left-over days from such hunks */
    /* The 0th of these years is the leap year */
    nyear = 0;				/* year in hunk */

    if (nrem > 366-1)			/* check for beyond leap year */
	nyear = (nrem-366)/365 + 1;	/* number of full years */

    *pyear += nquad*4 + nyear;		/* bump relative year */
    *pdays = nrem - 365*nyear - (nyear == 0 ? 0 : 1);
					/* get day number within year */
    return;
}
/* do100 -- handle a century
**
** This routine takes an ordinal day and figures out the relative
** year and day in year, assuming the simple-minded approach that
** years on 100 year boundaries are not leap years.  The zero-th
** year is assumed to be divisible by 100.
*/

static void
do100(pdays, pyear)
long * pdays;
int * pyear;
{
    int ncentury = *pdays / (100*365L + 100/4 - 1);
					/* number of centuries covered */
    *pdays %= (100*365L + 100/4 - 1);	/* get new number of days */
    *pyear += 100 * ncentury;		/* bump year */
    doleap(pdays, pyear, 4);		/* handle the rest for leap years;
					** note we won't hit the century
					** boundary anomaly; first following
					** leap year is 4 away
					*/
    return;
}
/* do400 -- handle clumps of 400 years
**
** This routine takes an ordinal day and finds the relative year and
** day within year assuming we have to deal with 100 and 400 year
** boundary funnies.  It also takes care of leap years.  The current
** zero-year must be divisible by 400.
*/

static void
do400(pdays, pyear)
long * pdays;
int * pyear;
{
    int n100 = 0;			/* number of centuries */
    int n400 = *pdays / (400*365L + 400/4 - 4 + 1);
					/* number of 400 year clumps */
    
    *pdays %= (400*365L + 400/4 - 4 + 1);
					/* update number of days */

    /* First century of tetra-centennial has all its leap years */

    if (*pdays > (100*365L + 100/4))
    {
	n100 = (*pdays - (100*365L + 100/4)) / (100*365L + 100/4 - 1) + 1;
					/* number of centuries */

	*pdays -= n100 * (100*365L + 100/4 - 1) + 1;
					/* gives day in century */
    }
    *pyear += 100 * n100 + 400 * n400;	/* update year */
    doleap(pdays, pyear, (n100 == 0) ? 0 : 4);
					/* handle century this way; first
					** leap year is current year if 400
					** boundary, else 4 away
					*/
    return;
}
/* to_day -- convert date to ordinal day
**
** This routine converts a date in the specified range to
** an ordinal day, subject to the constraints above.
** It returns -1L for a bad date.
*/

long
to_day(m, d, y)
int m;					/* month (1-12) */
int d;					/* day of month (1-31) */
int y;					/* year (1-9999) */
{
    int * upto = norm_upto;		/* array for normal months */
    long day;				/* ordinal day */

    /* check inputs */

    if (   y < 1 || y > 9999
	|| m < 1 || m > 12
	|| d < 1 || d > 31
	)
	return( -1L );

    day = (y-1) * 365L;			/* Well, most years have at least
					** this many days
					*/
    day += (y-1)/4;			/* account for all leap years */
    if (y > 1800)			/* take care of post-Gregorian
					** correction
					*/
    {
	day -= (y-1701)/100;		/* less 1 day per century */
	day += (y-1601)/400;		/* but one more per 400 years */
    }

    /* Now choose table for months and such */

    if (y < 1752)			/* pre strange year */
    {
	if (y % 4 == 0)			/* choose leap year table */
	    upto = leap_upto;
    }
    else if (y > 1752)			/* after strange year */
    {
	if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))
	    upto = leap_upto;		/* new leap year */
	day -= 11;			/* account for 1752 strangeness */
    }
    else				/* year must be 1752 */
    {
	upto = y1752_upto;		/* use special table */
	if (m == 9)
	{
	    if (d > 2 && d < 14)
		return( -1L );		/* illegal day */
	    else if (d >= 14)		/* fake day of month */
		d -= 11;
	}
    }
    if ( d > (upto[m+1] - upto[m]))	/* check for bad day of month */
	return( -1L );

    return( day + upto[m] + d );	/* return ordinal day */
}
