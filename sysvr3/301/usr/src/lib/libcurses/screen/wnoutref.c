/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wnoutref.c	1.4"
/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

# include	"curses.ext"

/* Put out window but don't actually update screen. */
wnoutrefresh(win)
register WINDOW	*win;
{
	register int copycnt, temp;
	register int wy, y;
	register chtype	*nsp;

	if (win == NULL)
		return ERR;

# ifdef DEBUG
	if(outf)
		if (win == stdscr)
			(void) fprintf(outf, "REFRESH(stdscr %x)", win);
		else if (win == curscr)
			(void) fprintf(outf, "REFRESH(curscr %x)", win);
		else
			(void) fprintf(outf, "REFRESH(%x)", win);
	if(outf) fprintf(outf, " (win == curscr) = %d, maxy %d\n", win, (win == curscr), win->_maxy);
	if (win != curscr)
		_dumpwin(win);
	if(outf) fprintf(outf, "REFRESH:\n\tfirstch\tlastch\n");
# endif
	/*
	 * initialize loop parameters
	 */

	if (win->_clear || win == curscr || SP->doclear) {
# ifdef DEBUG
		if (outf) fprintf(outf, "refresh clears, win->_clear %d, curscr %d\n", win->_clear, win == curscr);
# endif
		SP->doclear = 1;
		win->_clear = FALSE;
		if (win != curscr)
			touchwin(win);
	}

	if (win == curscr) {
		_ll_refresh(FALSE);
		return OK;
	}

	if (win->_flags & _WINCHANGED) {
		for (wy = 0; wy < win->_maxy; wy++) {
			if (win->_firstch[wy] != _NOCHANGE) {
				y = wy + win->_begy + win->_yoffset;
				nsp = &win->_y[wy][0];
				_ll_move(y, win->_begx);
				copycnt = win->_maxx;
				temp = columns - SP->virt_x;
				if (temp < copycnt) copycnt = temp;
				if (copycnt > 0)
					memcpy ((char *)SP->curptr,
						(char *)nsp,
						copycnt*sizeof(*nsp));
				SP->curptr += copycnt;
				SP->virt_x += copycnt;
				win->_firstch[wy] = _NOCHANGE;
			}
		}
		win->_flags &= ~_WINCHANGED;
	}
	win->_flags &= ~_WINMOVED;

	SP->leave_lwin = win;
	SP->leave_leave = win->_leave;
	SP->leave_use_idl |= win->_use_idl==2 ? win->_need_idl : win->_use_idl;
	SP->leave_x = win->_curx + win->_begx;
	SP->leave_y = win->_cury + win->_begy + win->_yoffset;
	return OK;
}
