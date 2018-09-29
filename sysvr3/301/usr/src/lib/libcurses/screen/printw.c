/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/printw.c	1.4"
/*
 * printw and friends
 *
 */

# include	"curses.ext"
# include	<varargs.h>

/*
 *	This routine implements a printf on the standard screen.
 */
/* VARARGS */
printw(va_alist)
va_dcl
{
	va_list ap;
	register char * fmt;

	va_start(ap);
	fmt = va_arg(ap, char *);
	return vwprintw(stdscr, fmt, ap);
}
