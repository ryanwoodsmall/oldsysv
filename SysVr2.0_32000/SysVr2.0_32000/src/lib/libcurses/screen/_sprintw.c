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
/*	@(#) _sprintw.c: 1.1 10/15/83	(1.4	3/1/83)	*/

# include	"curses.ext"
# include	<varargs.h>

/*
 *	This routine actually executes the printf and adds it to the window
 *
 *	This code now uses the vsprintf routine, which portably digs
 *	into stdio.  We provide a vsprintf for older systems that don't
 *	have one.
 */

_sprintw(win, fmt, ap)
WINDOW	*win;
char	*fmt;
va_list ap;
{
	char	buf[BUFSIZ];

	vsprintf(buf, fmt, ap);
	return waddstr(win, buf);
}
