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
/*
 * Simulation of termcap using terminfo.
 */

#include "curses.ext"

/*	@(#) tgetent.c: 1.1 10/15/83	(1.8	3/6/83)	*/

int
tgetent(bp, name)
char *bp, *name;
{
	int rv;

	if (setupterm(name, 1, &rv) >= 0)
		/* Leave things as they were (for compatibility) */
		reset_shell_mode();
	return rv;
}
