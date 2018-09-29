/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/getkey.c	1.1"

#include "sys/types.h"

long			getkey ()
{
	static int		started = 0;

	if (!started) {
		srand48 (time((time_t *)0));
		started = 1;
	}
	return (lrand48());
}
