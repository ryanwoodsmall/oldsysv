/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)setclk:setclk.c	1.3"
#include <stdio.h>
#include "sys/firmware.h"
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/nvram.h"
#include "sys/sys3b.h"
#include "time.h"

#define	dysize(A) (((A)%4)? 365: 366)	/* number of days per year */

static int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int month, day, year, hour, mminute, second, dayweek;
char weekday[4];
long timbuf;
int daylgt, tzone;

char *days[] = {
	"N",
	"sun",
	"mon",
	"tue",
	"wed",
	"thu",
	"fri",
	"sat"
};

main()
{
	char buffer[80];	/* buffer for reading input from terminal */
	char nv_data[2];	/* buffer for holding data read from nvram */
	char *nv_addr;		/* address of clock data in nvram */
	char  hw_month;
	struct nvparams nvp;
	struct todc clk;
	char dummy;
	int c;

	nv_addr = (char *) &(UNX_NVR->nv_month);
	nvp.addr = nv_addr;
	nvp.data = nv_data;
	nvp.cnt = 2;
	sys3b(RNVR, &nvp, 0);

	if (nv_data[0] == 0) {	/* time-of-day clock is no good */
		printf("\n		Time of Day Clock needs Restoring:\n");
		printf("		Change using \"sysadm datetime\" utility\n");
		return(0);
	}
	else {
		sys3b(RTODC, &clk, 0);
		hw_month = (char) clk.hmonth;
		if (nv_data[0] != hw_month) {
			if (nv_data[0] > hw_month) {
				nv_data[1]++;	/* increment year */
			}
			nv_data[0] = hw_month;
			sys3b(WNVR, &nvp, 0);
		}

		/* set unix clock from hw with year from nvram */
		htime((int)nv_data[1], &clk);
		sys3b(STIME, timbuf);
	}
}


newtime()
{
	register int i;

	if (month < 1 || month > 12) {
		printf("INVALID MONTH -- MUST BE BETWEEN 1 AND 12\n");
		return(1);
	}
	if (day < 1 || day > 31) {
		printf("INVALID DAY -- MUST BE BETWEEN 1 AND 31\n");
		return(1);
	}
	if (year < 70) {
		year += 2000;
	}
	else {
		year += 1900;
	}
	if (hour < 0 || hour > 23) {
		printf("INVALID HOUR -- MUST BE BETWEEN 0 AND 23\n");
		return(1);
	}
	if (mminute < 0 || mminute > 59) {
		printf("INVALID MINUTE -- MUST BE BETWEEN 0 AND 59\n");
		return(1);
	}
	if (second < 0 || second > 59) {
		printf("INVALID SECOND -- MUST BE BETWEEN 0 AND 59\n");
		return(1);
	}
	for (dayweek = 1; dayweek <= 7; dayweek++) {
		if (!strncmp(weekday, days[dayweek], 3) ) {
			break;
		}
	}
	if (dayweek > 7) {
		printf("INVALID DAY OF THE WEEK -- MUST BE ONE OF THE FOLLOWING:\n");
		printf("mon  tue  wed  thu  fri  sat  sun\n");
		return(1);
	}

	/* Calculate total number of days */
	timbuf = 0;
	for (i = 1970; i < year; i++) {
		timbuf += dysize(i);
	}
	if (dysize(year) == 366 && month >= 3) {/* current year is leap year */
		timbuf += 1;
	}
	for (i = month - 1; i > 0; i--) {
		timbuf += dmsize[i-1];
	}
	timbuf += (day-1);

	timbuf *= 24;	/* now hours */
	timbuf += hour;
	timbuf *= 60;	/* now minutes */
	timbuf += mminute;
	timbuf *= 60;	/* now seconds */
	timbuf += second;

		/* convert to Greenwich time, on assumption of Standard time. */
	timbuf += tzone*60*60;

		/* Now fix up to local daylight time. */
	if (daylgt) {
		timbuf += -1*60*60;
	}

	return(0);
}

/*
 * htime() - converts hardware time-of-day clock to number of
 * seconds, using the year parameter as the current year.
 */

htime(year, pclk)
register int year;
register struct todc *pclk;
{
	register int i, month;

	/* Calculate total number of days */
	timbuf = 0;

	if (year < 70) {
		year += 2000;
	}
	else {
		year += 1900;
	}

	for (i = 1970; i < year; i++) {
		timbuf += dysize(i);
	}

	month = pclk->hmonth;

	if (dysize(year) == 366 && month >= 3) {/* current year is leap year */
		timbuf += 1;
	}

	while(--month) {
		timbuf += dmsize[month-1];
	}

	timbuf += pclk->hdays - 1;

	timbuf *= 24;	/* now hours */
	timbuf += pclk->hhours;
	timbuf *= 60;	/* now minutes */
	timbuf += pclk->hmins;
	timbuf *= 60;	/* now seconds */
	timbuf += pclk->hsecs;
}
