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
/*	@(#) clearok.c: 1.1 10/15/83	(1.19	3/17/83)	*/

clearok(win,bf)	
WINDOW *win;
int bf;
{
#ifdef DEBUG
	if (win == stdscr)
		fprintf("it's stdscr: ");
	if (win == curscr)
		fprintf("it's curscr: ");
	if (outf) fprintf(outf, "clearok(%x, %d)\n", win, bf);
#endif
	if (win==curscr)
		SP->doclear = 1;
	else
		win->_clear = bf;
}
