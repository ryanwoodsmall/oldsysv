/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:dconv.c	1.3"
/* dconv.c
**
**	Convert from date to ordinal day and vice versa.
**	Convert input date and time to UNIX system time format.
**
** This file contains four conversion routines.  The first
** takes a date between 1/1/1 and 12/31/9999 and returns
** its ordinal number.  The second performs the inverse
** operation.  The encoding is based on the English
** calendar (and the "cal" program) and handles these
** anomalies:
**
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
**
** The last two conversion routines provide a numeric and a
** string day of the week, given an absolute date.
*/

#include <time.h>
/* These tables are useful for the conversion process */

/* Number of days up to first of each month for various types of year. */

/* Normal non-leap year */

int norm_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 59,				/* Jan, Feb, Mar */
    90, 120, 151,			/* Apr, May, Jun */
    181, 212, 243,			/* Jul, Aug, Sep */
    273, 304, 334,			/* Oct, Nov, Dec */
    365					/* end of year */
};

/* Normal leap year */

int leap_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 60,				/* Jan, Feb, Mar */
    91, 121, 152,			/* Apr, May, Jun */
    182, 213, 244,			/* Jul, Aug, Sep */
    274, 305, 335,			/* Oct, Nov, Dec */
    366					/* end of year */
};

/* That odd-ball year, 1752.  Note September! */

int y1752_upto[] =
{ 0,					/* so months run 1-12 */
    0, 31, 60,				/* Jan, Feb, Mar */
    91, 121, 152,			/* Apr, May, Jun */
    182, 213, 244,			/* Jul, Aug, Sep */
    263, 294, 324,			/* Oct, Nov, Dec */
    355					/* end of year */
};

/* Table of names of days.  Day 1 is a Saturday. */

static char * dayname[] =
{ "Friday", "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday" };
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
/* weekday -- return numeric day of the week
**
** This routine returns the numeric day of the week that corresponds
** to an absolute date.  Output day 0 is a Sunday.  Input day 1 is a
** Saturday.
*/

int
weekday(day)
long day;				/* absolute date */
{
    return( (day+5) % 7 );		/* day 1 is a Saturday */
}
/* sweekday -- return string name of day of the week
**
** This routine returns the name of the day of the week that
** corresponds to an absolute date.  Day 1 is Saturday.  The
** static array therefore begins with Friday at [0].
*/

char *
sweekday(day)
long day;				/* absolute date */
{
    return( dayname[day%7] );
}
/* tconv -- time convert
**
** This routine converts year, month, day, hour, minute, second
** to UNIX's internal time format.  If the input is bad,
** -1 is returned.
*/

long
tconv(year, month, day, hour, minute, second)
int year, month, day;			/* date parameters */
int hour, minute, second;		/* time parameters */
{
    long time = 0;			/* initial time */
    long date = to_day(month, day, year);
					/* get date (-1 if bad) */
    
    if (date < 0)
	return( -1L );			/* indicate bad input */

    /* the last time UNIX can handle is 1/18/2038; for simplicity,
    ** stop at 2038
    */
    if (year < 1970 || year > 2037)	/* UNIX can't handle these */
	return( -1L );

    time = date - to_day(1, 1, 1970);	/* day zero is 1/1/1970 */
    if (0 <= hour && hour <= 23)
	time = time * 24 + hour;
    else
	return( -1L );			/* bad hour */
    
    if (0 <= minute && minute <= 59)
	time = time * 60 + minute;
    else
	return( -1L );			/* bad minute */
    
    if (second < 0 || second >= 60)
	return( -1L );			/* bad seconds */

    time = time * 60 + second;		/* include seconds */

    tzset();				/* set time zone information */
    time += timezone;			/* adjust time for timezone */

    /* Correct for daylight savings time.  Since the number of timezone
    ** hours is one larger during DST, we must subtract an hour's worth
    ** of seconds to compensate (since the GMT was unchanged).
    */

    if (localtime(&time)->tm_isdst != 0)
	time -= 1*60*60;

    return( time );
}
/* tconvs -- convert time string
**
** This routine converts a time string of the sort used by
** the date(1) command to UNIX internal time format,
** which is seconds since midnight, 1/1/1970.
** The time string is of the form
**	mmddhhmm[yy]
** which stands for month-day-hour-minute[-year]
** If the optional year is < 70, we assume it's after 2000.
** Returns -1 if the date/time string is bad.
*/

/* convert two digit chars to number */
#define	cvt(d1,d2)	(((d1) - '0')*10+d2-'0')

long
tconvs(s)
char * s;				/* input string */
{
    char * cp = s;
    int len = 0;
    int year;
    long today = time((long *) 0);	/* get current time */
    struct tm * curtime = localtime(&today);
    long time;

    /* check for all digits and a reasonable string length */

    while (*cp != '\0')
    {
	len++;
	if (*cp < '0' || *cp > '9')
	    return( -1L );		/* bad string */
	cp++;
    }
    if (len != 8 && len != 10)
	return( -1L );			/* bad length */

    /* tconv takes year, month, day, hour, minute, second */

    if (len == 10)
    {
	if ((year = cvt(s[8], s[9]) + 1900) < 1970)
	    year += 100;		/* make years < 1970 into next century */;
    }
    else
	year = 1900 + curtime->tm_year;

    time =				/* set preliminary time */
	tconv(year, cvt(s[0], s[1]), cvt(s[2], s[3]),
		cvt(s[4], s[5]), cvt(s[6], s[7]), 0);
    
    return( time );
}
