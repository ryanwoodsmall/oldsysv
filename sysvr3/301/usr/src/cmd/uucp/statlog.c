/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uucp:statlog.c	1.1"

#include "uucp.h"

/*
	Report and log file transfer rate statistics.
	This is ugly because we are not using floating point.
*/

void
statlog( direction, bytes, millisecs )
char		*direction;
unsigned long	bytes;
time_t		millisecs;
{
	char		text[ 100 ];
	unsigned long	bytes1000;

	bytes1000 = bytes * 1000;
	(void) sprintf(text, "%s %lu / %lu.%.3u secs, %lu bytes/sec",
		direction, bytes, millisecs/1000, millisecs%1000,
		bytes1000/millisecs );
	CDEBUG(4, "%s\n", text);
	syslog(text);
}
