/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/clearok.c	1.1"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

clearok(win,bf)
WINDOW *win;
int bf;
{
#ifdef DEBUG
	if (outf) {
		if (win == stdscr)
			fprintf(outf, "it's stdscr: ");
		if (win == curscr)
			fprintf(outf, "it's curscr: ");
		fprintf(outf, "clearok(%x, %d)\n", win, bf);
	}
#endif
	if (win==curscr)
		SP->doclear = 1;
	else
		win->_clear = bf;
}
