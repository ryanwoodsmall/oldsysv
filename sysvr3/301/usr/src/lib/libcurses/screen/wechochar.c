/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wechochar.c	1.1"
#include	"curses.ext"
extern int _ll_echochar();

/*
 *  These routines short-circuit much of the innards of curses in order to get
 *  a single character output to the screen quickly! It is used by getch()
 *  and getstr().
 *
 *  wechochar(WINDOW *win, chtype ch) is functionally equivalent to
 *  waddch(WINDOW *win, chtype ch), wrefresh(WINDOW *win)
 */

int
wechochar (win, ch)
register WINDOW *win;
chtype ch;
{
	register int wy = win->_cury, wx = win->_curx;
	register int SPy = wy + win->_begy + win->_yoffset;
	register int SPx = wx + win->_begx;
	register chtype c = ch | win->_attrs,
		rawc = ch & A_CHARTEXT,
		attrs = c & A_ATTRIBUTES;

	/* Check for reasons that force us to do a waddch(). */
	if ((rawc < ' ' || rawc >= 0177) ||	/* control char */
	    (wx >= win->_maxx - 1) ||		/* right edge of window */
	    (SP->cur_body [SPy+1] == NULL) ||	/* no line structure */
	    (win->_flags&_WINCHANGED) ) {	/* no recent refresh() */
		(void) waddch (win, (chtype) ch);
		wnoutrefresh (win);
		return doupdate ();
	}

	/* waddch(): Handle the virtual window. */
	win->_y [wy][wx] = c;
	win->_curx++;

	/* wnoutrefresh(): Handle the virtual screen. */
	/* Check for reasons that force us to do a wnoutrefresh(). */
	if (win->_clear || SP->doclear ||/* screen has been cleared */
	    magic_cookie_glitch > 0 ||	/* need more than can be done here */
	    SP->fl_changed ||		/* no recent doupdate() */
	    (SP->slk && SP->slk->fl_changed)) {
		win->_firstch [wy] = win->_lastch [wy] = wx;
		win->_flags |= _WINCHANGED;
		wnoutrefresh (win);
		return doupdate ();
	}

	/* doupdate: Handle the physical screen. */

	/* ll_refresh(): Do the physical writing. */
	return _ll_echochar(SPy, SPx, c, attrs, rawc);
}
