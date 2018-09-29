/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)date:date.c	1.13"
/*
**	date - with format capabilities and international flair
*/

#include        <stdio.h>
#include	<time.h>
#include	<sys/types.h>
#include	"utmp.h"
#define	year_size(A)	(((A) % 4) ? 365 : 366)

char		buf[100];
long		clock;
static short	month_size[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
struct utmp	wtmp[2] = { {"", "", OTIME_MSG, 0, OLD_TIME, 0, 0, 0}, 
			    {"", "", NTIME_MSG, 0, NEW_TIME, 0, 0, 0} };

main(argc, argv)
int	argc;
char	**argv;
{
	long	time(), lseek();
	int	status;


	/*  Initialize variables  */
	status = 0;
	time(&clock);

	switch(argc) {
	case 1:
		cftime(buf, "", &clock);
		puts(buf); 
		break;
	case 2:
		if(argv[1][0] == '+')
			{
			cftime(buf, &argv[1][1], &clock);
			puts(buf);
			}
		else
			status = setdate(localtime(&clock), argv[1]);
		break;
	default:
		(void) fprintf(stderr, "usage: date [ mmddhhmm[ccyy | yy] ]  [ +format ]\n");
		status = 2;
		break;
	}
	exit(status);
}

setdate(current_date, date)
struct tm	*current_date;
char		*date;
{
	register int i;
	int mm, dd, hh, min, yy, wf;

	/*  Parse date string  */
	switch(strlen(date)) {
	case 12:
		yy = atoi(&date[8]);
		date[8] = '\0';
		break;
	case 10:
		yy = 1900 + atoi(&date[8]);
		date[8] = '\0';
		break;
	case 8:
		yy = 1900 + current_date->tm_year;
		break;
	default:
		(void) fprintf(stderr, "date: bad conversion\n");
		return(1);
	}
	min = atoi(&date[6]);
	date[6] = '\0';
	hh = atoi(&date[4]);
	date[4] = '\0';
	dd = atoi(&date[2]);
	date[2] = '\0';
	mm = atoi(&date[0]);
	if(hh == 24)
		hh = 0, dd++;

	/*  Validate date elements  */
	if(!((mm >= 1 && mm <= 12) && (dd >= 1 && dd <= 31) &&
		(hh >= 0 && hh <= 23) && (min >= 0 && min <= 59))) {
		(void) fprintf(stderr, "date: bad conversion\n");
		return(1);
	}

	/*  Build date and time number  */
	for(clock = 0, i = 1970; i < yy; i++)
		clock += year_size(i);
	/*  Adjust for leap year  */
	if (year_size(yy) == 366 && mm >= 3)
		clock += 1;
	/*  Adjust for different month lengths  */
	while(--mm)
		clock += month_size[mm - 1];
	/*  Load up the rest  */
	clock += (dd - 1);
	clock *= 24;
	clock += hh;
	clock *= 60;
	clock += min;
	clock *= 60;

	/* convert to GMT assuming standard time */
	/* correction is made in localtime(3C) */

	clock += timezone;

	/* correct if daylight savings time in effect */

	if (localtime(&clock)->tm_isdst)
		clock = clock - (timezone - altzone); 

	/*  Print the new date and time  */
	cftime(buf, "", &clock);
	puts(buf);

	(void) time(&wtmp[0].ut_time);
	if(stime(&clock) < 0) {
		(void) fprintf(stderr, "date: no permission\n");
		return(1);
	}
	(void) time(&wtmp[1].ut_time);
	pututline(&wtmp[0]);
	pututline(&wtmp[1]);
	if ((wf = open(WTMP_FILE, 1)) >= 0) {
		(void) lseek(wf, 0L, 2);
		(void) write(wf, (char *) wtmp, sizeof(wtmp));
	}
	return(0);
}
