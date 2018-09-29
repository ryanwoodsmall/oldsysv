/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/addch.c	1.4"
# include	"curses.ext"

/*
 *	This routine prints the character in the current position.
 *	Think of it as putc.
 *
 */
waddch(win, c)
register WINDOW	*win;
register chtype		c;
{
	register int		x, y;
	char *uctr;
	register char rawc = c & A_CHARTEXT;

	x = win->_curx;
	y = win->_cury;
# ifdef DEBUG
	if (outf)
		if (c == rawc)
			fprintf(outf, "'%c'", rawc);
		else
			fprintf(outf, "'%c' %o, raw %o", c, c, rawc);
# endif
	if (y >= win->_maxy || x >= win->_maxx || y < 0 || x < 0) {
# ifdef DEBUG
		if(outf) fprintf(outf, "off edge, (%d,%d) not in (%d,%d)\n",
			y, x, win->_maxy, win->_maxx);
# endif
		return ERR;
	}
	switch (rawc) {
	  case '\t':
	  {
		register int newx;
		register chtype space = ' ' | (c & A_ATTRIBUTES);

		for (newx = x + (8 - (x & 07)); x < newx; x++)
			if (waddch(win, space) == ERR)
				return ERR;
		return OK;
	  }
	  default:
		if (rawc < ' ' || rawc > '~') {
			register chtype attrs = c & A_ATTRIBUTES;
			uctr = unctrl(rawc);
			if (waddch(win, (chtype)uctr[0]|attrs) == ERR)
				return ERR;
			return waddch(win, (chtype)uctr[1]|attrs);
		}
#ifdef DEBUG
		if (win->_attrs) {
			if(outf) fprintf(outf, "(attrs %o, %o=>%o)", win->_attrs, c, c | win->_attrs);
		}
#endif
		c |= win->_attrs;
		if (win->_y[y][x] != c) {
			if (win->_firstch[y] == _NOCHANGE)
				win->_firstch[y] = win->_lastch[y] = x;
			else if (x < win->_firstch[y])
				win->_firstch[y] = x;
			else if (x > win->_lastch[y])
				win->_lastch[y] = x;
		}
		win->_y[y][x++] = c;
		if (x >= win->_maxx) {
new_line:
			x = 0;
			if (++y > win->_bmarg) {
				if (win->_scroll && !(win->_flags&_ISPAD)) {
					wrefresh(win);
					win->_need_idl = TRUE;
					_tscroll(win, 0);
					--y;
				}
				else {
# ifdef DEBUG
					if(outf) {
					    int i;
					    fprintf(outf,
					    "ERR because (%d,%d) > (%d,%d)\n",
					    x, y, win->_maxx, win->_maxy);
					    fprintf(outf, "line: '");
					    for (i=0; i<win->_maxy; i++)
						fprintf(outf, "%c",
							win->_y[y-1][i]);
					    fprintf(outf, "'\n");
					}
# endif
					win->_flags |= _WINCHANGED;
					return ERR;
				}
			}
		}
# ifdef FULLDEBUG
		if(outf) fprintf(outf, "ADDCH: 2: y = %d, x = %d, firstch = %d, lastch = %d\n", y, x, win->_firstch[y], win->_lastch[y]);
# endif
		break;
	  case '\n':
		wclrtoeol(win);
		goto new_line;
	  case '\r':
		x = 0;
		break;
	  case '\b':
		if (--x < 0)
			x = 0;
		break;
	}
	win->_curx = x;
	win->_cury = y;
	win->_flags |= _WINCHANGED;
	return OK;
}
