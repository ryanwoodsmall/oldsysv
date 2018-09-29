/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/insch.c	1.4"
# include	"curses.ext"

/*
 *	This routine performs an insert-char on the line, leaving
 * (_cury,_curx) unchanged.
 *
 */
winsch(win, c)
register WINDOW	*win;
chtype		c;
{
	register chtype	*temp1, *temp2;
	register chtype	*end;
	register int blanks, y;
	register chtype space, rawc;

	switch (rawc = (c & A_CHARTEXT)) {
		case '\t':
			/* insert blanks */
			blanks = 8 - (win->_curx & 07);
			space = ' ' | (c & A_ATTRIBUTES);
			while (blanks--)
				winsch(win, space);
			break;
		case '\b':
		case '\r':
			waddch(win, c);
			break;
		case '\n':
			/* move to next line, possibly scrolling */
			y = win->_cury + 1;
			if (y > win->_bmarg) {
				if (win->_scroll && !(win->_flags&_ISPAD)) {
					wrefresh(win);
					win->_need_idl = TRUE;
					_tscroll(win, 0);
					--y;
				} else {
					win->_flags |= _WINCHANGED;
					return ERR;
				}
			}
			win->_curx = 0;
			win->_cury = y;
			win->_flags |= _WINCHANGED;
			break;
		default:
			/* for control chars, insert "^X" */
			if (rawc < ' ' || rawc > '~') {
				register chtype attrs = c & A_ATTRIBUTES;
				register char *uctr  = unctrl(rawc);

				if (winsch(win, (chtype)uctr[1]|attrs) == ERR)
					return ERR;
				return winsch(win, (chtype)uctr[0]|attrs);
			}

			/* insert regular chars */
			win->_flags |= _WINCHANGED;
			end = &win->_y[win->_cury][win->_curx];
			temp1 = &win->_y[win->_cury][win->_maxx - 1];
			temp2 = temp1 - 1;
			while (temp1 > end)
				*temp1-- = *temp2--;
			c |= win->_attrs;;
			*temp1 = c;
			win->_lastch[win->_cury] = win->_maxx - 1;
			if (win->_firstch[win->_cury] == _NOCHANGE ||
			    win->_firstch[win->_cury] > win->_curx)
				win->_firstch[win->_cury] = win->_curx;
			if (win->_cury == lines - 1 &&
			    win->_y[lines-1][COLS-1] != ' ')
				if (win->_scroll && !(win->_flags&_ISPAD)) {
					wrefresh(win);
					scroll(win);
					win->_cury--;
				} else
					return ERR;
	}
	return OK;
}
