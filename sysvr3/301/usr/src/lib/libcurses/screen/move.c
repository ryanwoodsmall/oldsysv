/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/move.c	1.3.1.1"
# include	"curses.ext"

/*
 *	This routine moves the cursor to the given point
 *
 */
int
wmove(win, y, x)
register WINDOW	*win;
register int	y, x;
{
# ifdef DEBUG
	if(outf) fprintf(outf, "MOVE to win ");
	if (win == stdscr)
		if(outf) fprintf(outf, "stdscr ");
	else
		if(outf) fprintf(outf, "%o ", win);
	if(outf) fprintf(outf, "(%d, %d)\n", y, x);
# endif
	if (x < 0 || y < 0 || x >= win->_maxx || y >= win->_maxy)
		return ERR;
	win->_curx = x;
	win->_cury = y;
	win->_flags |= _WINMOVED;
	return OK;
}
