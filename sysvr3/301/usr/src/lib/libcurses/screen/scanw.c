/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/scanw.c	1.4"
/*
 * scanw and friends
 *
 */

# include	"curses.ext"
# include	<varargs.h>

/*
 *	This routine implements a scanf on the standard screen.
 */
/* VARARGS */
scanw(va_alist)
va_dcl
{
	register char	*fmt;
	va_list	ap;

	va_start(ap);
	fmt = va_arg(ap, char *);
	return vwscanw(stdscr, fmt, ap);
}
