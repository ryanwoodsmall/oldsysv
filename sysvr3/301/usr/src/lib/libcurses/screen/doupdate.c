/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/doupdate.c	1.5.1.1"
/*
 * make the current screen look like "win" over the area covered by
 * win.
 */

# include	"curses.ext"

/* Update screen */
int
doupdate()
{
	int rc;
	int _outch();

	if (SP->leave_lwin == NULL)
		return ERR;

	if (SP->fl_endwin) {

		/*
		 * We've called endwin since last refresh.  Undo the
		 * effects of this call.
		 */

		_fixdelay((bool)FALSE, (bool)SP->fl_nodelay);
		if (stdscr->_use_meta)
			tputs(meta_on, 1, _outch);
		SP->fl_endwin = FALSE;

#ifdef _VR2_COMPAT_CODE
		{ extern int _endwin; _endwin = FALSE; }
#endif
		slk_touch();
		reset_prog_mode();
	}

	/* Tell the back end where to leave the cursor */
	_ll_endmove();
	if (SP->leave_leave) {
		_ll_setyx(-1, -1);
		if (SP->cursorstate != 0 && cursor_invisible) {
			tputs (cursor_invisible, 1, _outch);
			SP->cursorstate = 0;
		}
	} else {
		_ll_setyx(SP->leave_y, SP->leave_x);
		if (SP->cursorstate == 0) {
			if (hard_cursor && cursor_visible) {
				tputs (cursor_visible, 1, _outch);
				SP->cursorstate = 2;
			} else {
				tputs (cursor_normal, 1, _outch);
				SP->cursorstate = 1;
			}
		}
	}
	rc = _ll_refresh(SP->leave_use_idl);
#ifdef DEBUG
	_dumpwin(SP->leave_lwin);
#endif
	return rc;
}

#ifdef DEBUG
_dumpwin(win)
register WINDOW *win;
{
	register int x, y;
	register chtype *nsp;

	if (!outf)
		return;

	if (!win) {
		fprintf(outf, "_dumpwin(win==NULL)\n");
		return;
	}
	if (win == stdscr)
		fprintf(outf, "_dumpwin(stdscr)--------------\n");
	else if (win == curscr)
		fprintf(outf, "_dumpwin(curscr)--------------\n");
	else
		fprintf(outf, "_dumpwin(%o)----------------\n", win);
	for (y=0; y<win->_maxy; y++) {
		if (y > 76)
			break;
		nsp = &win->_y[y][0];
		fprintf(outf, "%d: ", y);
		for (x=0; x<win->_maxx; x++) {
			_sputc(*nsp, outf);
			nsp++;
		}
		fprintf(outf, "\n");
	}
	fprintf(outf, "end of _dumpwin----------------------\n");
}

static _sputc(c, f)
chtype c;
FILE *f;
{
	int so;

	so = c & A_ATTRIBUTES;
	c &= 0177;
	if (so) {
		putc('<', f);
		fprintf(f, "%o,", so);
	}
	putc(c, f);
	if (so)
		putc('>', f);
}
#endif /* DEBUG */
