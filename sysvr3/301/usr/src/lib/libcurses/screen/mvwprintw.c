/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mvwprintw.c	1.3.1.1"
# include	"curses.ext"
# include	<varargs.h>

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */

/* VARARGS */
mvwprintw(va_alist)
va_dcl
{
	va_list ap;
	register WINDOW	*win;
	register int	y, x;
	register char * fmt;

	va_start(ap);
	win = va_arg(ap, WINDOW *);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
	return wmove(win, y, x) == OK ? vwprintw(win, fmt, ap) : ERR;
}
