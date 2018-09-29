/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mvscanw.c	1.3.1.1"
# include	"curses.ext"
# include	<varargs.h>

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 */

/* VARARGS */
mvscanw(va_alist)
va_dcl
{
	register int	y, x;
	register char	*fmt;
	va_list		ap;

	va_start(ap);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
	return move(y, x) == OK ? vwscanw(stdscr, fmt, ap) : ERR;
}
