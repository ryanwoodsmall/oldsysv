/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/fdate.c	1.2"
/*	
	fdate [-format] file ...

		print file modification, creation or access date/time
		e.g., fdate '-.ds DT %D' file | nroff -mm - file


	This version is intended for UNIX/TS.

	Originally written by Ron A. Levenberg.
 */
#include "stdio.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"

char	*cmd;	/* name of this command */

main(argc, argv)
int	argc;
char	**argv;
{
	struct	stat	stbuf;
	char	*aptr;
	char	buf[200];
	extern struct tm *localtime();
	char	*fdate();

	cmd = *argv++; argc--;

	if (argc > 0 && **argv == '-') {
		aptr = *argv++ + 1; argc--;
	}else{
		aptr = "%U %f";
	}

	for ( ; argc > 0; argc--, argv++) {
		if (stat(*argv,&stbuf) == -1) {
			fprintf(stderr, "%s: %s non-existent\n", cmd,*argv);
			continue;
		}
		fdate(buf, aptr, stbuf.st_ctime, stbuf.st_mtime, stbuf.st_atime,*argv);
		fputs(buf, stdout);
	}
	exit (0);
}

#define	MONTH	itoa(timptr->tm_mon +1, tp, 2)
#define	DAY	itoa(timptr->tm_mday, tp, 2)
#define	YEAR	itoa(timptr->tm_year, tp, 2)
#define	HOUR	itoa(timptr->tm_hour, tp, 2)
#define	MINUTE	itoa(timptr->tm_min, tp, 2)
#define	SECOND	itoa(timptr->tm_sec, tp, 2)
#define	JULIAN	itoa(timptr->tm_yday +1, tp, 3)
#define	WEEKDAY	itoa(timptr->tm_wday, tp, 1)
#define	MODHOUR	itoa(h, tp, 2)

char	month[12][3] = {
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"};

char	days[7][3] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"};


char *
itoa(i, ptr, dig)
	register  int	i;
	register  int	dig;
	register  char	*ptr;
{
	switch(dig)	{
		case 3:
			*ptr++ = i/100 + '0';
			i = i - i / 100 * 100;
		case 2:
			*ptr++ = i / 10 + '0';
		case 1:	
			*ptr++ = i % 10 + '0';
	}
	return	(ptr);
}


char *
fdate(buf, aptr, ctim, mtim, atim, file)
char	buf[];
register	char	*aptr;
long	ctim;
long	mtim;
long	atim;
char	*file;
{
	struct	tm	*timptr;
	register	char	*tp;
	char	*fp;
	register	char	c;
	int	h;
	int	i;
	int	stopflg;

	stopflg = 0;
	timptr = localtime(&mtim);	/* modification time is default */
	tp = buf; *tp = '\0';
	while((!stopflg) && (c = *aptr++)) {
		if(c == '%')
			switch(*aptr++) {
			case '%':
				*tp++ = '%';
				continue;
			case 'n':
				*tp++ = '\n';
				continue;
			case 't':
				*tp++ = '\t';
				continue;
			case 'm' :
				tp = MONTH;
				continue;
			case 'd':
				tp = DAY;
				continue;
			case 'y':
				tp = YEAR;
				continue;
			case 'D':
				tp = MONTH;
				*tp++ = '/';
				tp = DAY;
				*tp++ = '/';
				tp = YEAR;
				continue;
			case 'H':
				tp = HOUR;
				continue;
			case 'M':
				tp = MINUTE;
				continue;
			case 'S':
				tp = SECOND;
				continue;
			case 'T':
				tp = HOUR;
				*tp++ = ':';
				tp = MINUTE;
				*tp++ = ':';
				tp = SECOND;
				continue;
			case 'j':
				tp = JULIAN;
				continue;
			case 'w':
				tp = WEEKDAY;
				continue;
			case 'r':
				h = timptr->tm_hour;
				if((h %= 12) == 0)  h = 12;
				tp = MODHOUR;
				*tp++ = ':';
				tp = MINUTE;
				*tp++ = ':';
				tp = SECOND;
				*tp++ = ' ';
				*tp++ = (timptr->tm_hour >= 12) ? 'P' : 'A';
				*tp++ = 'M';
				continue;
			case 'h':
				for(i=0; i<3; i++)
					*tp++ = month[timptr->tm_mon][i];
				continue;
			case 'a':
				for(i=0; i<3; i++)
					*tp++ = days[timptr->tm_wday][i];
				continue;
			case 'f':
				fp = file;
				while (*fp) *tp++ = *fp++;
				continue;
			case 'C':
				timptr = localtime(&ctim);
				continue;
			case 'W':
				timptr = localtime(&mtim);
				continue;
			case 'A':
				timptr = localtime(&atim);
				continue;
			case 'U':
				for(i=0; i<3; i++)
					*tp++ = days[timptr->tm_wday][i];
				*tp++ = ' ';
				for(i=0; i<3; i++)
					*tp++ = month[timptr->tm_mon][i];
				*tp++ = ' ';
				tp = DAY;
				*tp++ = ' ';
				tp = HOUR;
				*tp++ = ':';
				tp = MINUTE;
				*tp++ = ':';
				tp = SECOND;
				*tp++ = ' ';
				*tp++ = '1';
				*tp++ = '9';
				tp = YEAR;
				continue;
			case '\0':
				stopflg = 1;
				continue;
			default:
				*tp++ = *(aptr - 1);
				continue;
			}
		*tp++ = c;
	}
	*tp++ = '\n';
	*tp = '\0';
	return  buf;
}
