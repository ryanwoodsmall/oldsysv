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
# include	"curses.ext"
/*	@(#) scroll.c: 1.1 10/15/83	(1.3	7/17/82)	*/

/*
 *	This routine scrolls the window up a line.
 *
 * 7/8/81 (Berkeley) @(#)scroll.c	1.2
 */
scroll(win)
WINDOW *win;
{
	_tscroll(win, 1);
}
