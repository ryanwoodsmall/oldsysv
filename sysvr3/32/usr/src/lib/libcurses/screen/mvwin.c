/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mvwin.c	1.8"
#include	"curses_inc.h"

/* relocate the starting position of a _window */

mvwin(win, by, bx)
register	WINDOW	*win;
register	int	by, bx;
{
    if ((by + win->_yoffset + win->_maxy) > (lines - SP->Yabove) ||
            (bx + win->_maxx) > COLS || by < 0 || bx < 0)
         return ERR;
    win->_begy = by;
    win->_begx = bx;
    (void) wtouchln(win, 0, win->_maxy, -1);
    return (win->_immed ? wrefresh(win) : OK);
}
