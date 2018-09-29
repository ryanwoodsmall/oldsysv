/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/watch.c	1.2"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/times.h>
#include	"wish.h"

long	times();

/*
 * call stopwatch with "0" to "reset", and with a non-zero value
 * to print elapsed times on the stderr
 */
void
stopwatch(flag)
int	flag;
{
	static long	start;
	static struct tms	tbuf;

	if (flag == 0)
		start = times(&tbuf);
	else {
		long	stop;
		struct tms	stoptim;

		stop = times(&stoptim);
#ifdef _DEBUG0
		_debug0(stderr, "Real %d.%02d, User %d.%02d, System %d.%02d\n",
			(stop - start) / 100, (stop - start) % 100,
			(stoptim.tms_utime - tbuf.tms_utime) / 100,
			(stoptim.tms_utime - tbuf.tms_utime) % 100,
			(stoptim.tms_stime - tbuf.tms_stime) / 100,
			(stoptim.tms_stime - tbuf.tms_stime) % 100);
#endif
	}
}
