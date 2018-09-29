/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include "curses.ext"
/*	@(#) _fixdelay.c: 1.1 10/15/83	(1.19	3/17/83)	*/

/*
 * The use has just changed his notion of whether we want nodelay mode.
 * Do any system dependent processing.
 */
/* ARGSUSED */
_fixdelay(old, new)
bool old, new;
{
#ifdef USG
# include <fcntl.h>
	int fl, rv, fd;
	extern int errno;
	FILE	*inf;

	inf = SP -> term_file;
	if( inf == stdout )
	{
		inf = stdin;
	}
	fd = fileno( inf );
	fl = fcntl(fd, F_GETFL, 0);
	if (new)
		fl |= O_NDELAY;
	else
		fl &= ~O_NDELAY;
	if (old != new)
		rv = fcntl(fd, F_SETFL, fl);
#else
	/* No system dependent processing on the V7 or Berkeley systems. */
#endif
}
