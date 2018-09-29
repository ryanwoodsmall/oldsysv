/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)calendar:calprog.c	1.8"
/* /usr/lib/calprog produces an egrep -f file
   that will select today's and tomorrow's
   calendar entries, with special weekend provisions

   used by calendar command
*/


#include <time.h>

#define DAY (3600*24L)
long time();

static
char *month[] = {
	"[Jj]an",
	"[Ff]eb",
	"[Mm]ar",
	"[Aa]pr",
	"[Mm]ay",
	"[Jj]un",
	"[Jj]ul",
	"[Aa]ug",
	"[Ss]ep",
	"[Oo]ct",
	"[Nn]ov",
	"[Dd]ec"
};
struct tm *localtime();

static
tprint(t)
long t;
{
	struct tm *tm;
	tm = localtime(&t);
	(void)printf("(^|[ \t(,;])((%s[^ ]* *|0*%d/|\\*/)0*%d)([^0123456789]|$)\n",
		month[tm->tm_mon], tm->tm_mon + 1, tm->tm_mday);
}

main()
{
	long t;

	(void)time(&t);
	tprint(t);
	switch(localtime(&t)->tm_wday) {
	case 5:
		t += DAY;
		tprint(t);
	case 6:
		t += DAY;
		tprint(t);
	default:
		t += DAY;
		tprint(t);
	}
	exit(0);
}
