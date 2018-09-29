/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mvwscanw.c	1.4.1.1"
# include	"curses.ext"
# include	<varargs.h>

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 */

/* VARARGS */
mvwscanw(va_alist)
va_dcl
{
	register WINDOW	*win;
	register int	y, x;
	register char	*fmt;
	va_list ap;

	va_start(ap);
	win = va_arg(ap, WINDOW *);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
	return wmove(win, y, x) == OK ? vwscanw(win, fmt, ap) : ERR;
}
