/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xt:xtt.c	2.3"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	Print out Xt driver traces
 */

#include "sys/types.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/xtproto.h"
#include "sys/xt.h"
#include "stdio.h"

char *name;
int display;
int traceoff;
extern void xtraces();
extern char _sobuf[];
char usage[] = "Usage: %s [-o] [-f]\n";

main(argc, argv)
char *argv[];
{
	name = *argv++;
	for (; --argc; argv++) {
		if (argv[0][0] == '-' && argv[0][2] == 0)
			switch (argv[0][1]) {
			case 'f':
				display++;
				break;
			case 'o':
				traceoff++;
				break;
			default:
				(void)fprintf(stderr, usage, name);
				return 1;
			}
		else {
			(void)fprintf(stderr, usage, name);
			return 1;
		}
	}
	setbuf(stdout, _sobuf);
	xtraces(0, stdout);
	if (display)
		(void)fprintf(stdout, "\f");
	if (traceoff)
		(void)ioctl(0, XTIOCNOTRACE, 0);
	return 0;
}
