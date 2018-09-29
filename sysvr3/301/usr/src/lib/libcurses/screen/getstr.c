/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getstr.c	1.5.1.1"
# include	"curses.ext"

/*
 *	This routine gets a string starting at (_cury,_curx)
 *
 */
wgetstr(win,str)
WINDOW	*win;
char	*str;
{
	int i, total = 0;
	char myerase, mykill;
	char rownum[256], colnum[256], length[256];
	int doecho = SP->fl_echoit;
	int savecb = SP->fl_rawmode;
	register int cpos = 0;
	register int ch;
	register char *cp = str;

#ifdef DEBUG
	if (outf) fprintf(outf, "doecho %d, savecb %d\n", doecho, savecb);
#endif

	myerase = erasechar();
	mykill = killchar();
	if (doecho)	noecho();
	if (!savecb)	cbreak();

	for (;;) {
		rownum[cpos] = win->_cury;
		colnum[cpos] = win->_curx;
		ch = wgetch(win);
		if (ch <= 0 ||ch == ERR ||
		    ch == '\n' || ch == '\r' ||
		    ch == KEY_ENTER)
			break;
		else if (ch == myerase || ch == KEY_LEFT ||
			 ch == KEY_BACKSPACE) {
			if (cpos > 0) {
				cp--; cpos--;
				if (doecho) {
					wmove(win, rownum[cpos], colnum[cpos]);
			/* Add the correct amount of blanks. */
					for (i = length[cpos]; i > 0; i--)
					    waddch (win,' ');
			/* Move back after the blanks are put in. */
					wmove(win, rownum[cpos], colnum[cpos]);
					wrefresh(win);
			/* Update total. */
					total-=length[cpos];
				}
			}
		} else if (ch == mykill) {
			cp = str;
			cpos = 0;
			if (doecho) {
				wmove(win, rownum[cpos], colnum[cpos]);
			/* Add the correct amount of blanks. */
				for (i = total; i > 0; i--)
				    waddch (win,' ');
			/* Move back after the blanks are put in. */
				wmove(win, rownum[cpos],colnum[cpos]);
				wrefresh(win);
			/* Update total. */
				total = 0;
			}
		} else if (ch >= KEY_BREAK) {
			beep();
		} else {
			*cp++ = ch;
			if (doecho) {
				/* Add the length of the */
				/* character to total. */
				if ((ch >= ' ') && (ch < 0177))
					length[cpos] = 1;
				else if (ch == '\t')
					length[cpos] = 8 - (colnum[cpos] & 07);
				else
					length[cpos] = 2;
				total += length[cpos];
				wechochar(win, (chtype) ch);
			}
			cpos++;
		}
	}

	*cp = '\0';

	if (doecho)
		echo();
	if (!savecb)
		nocbreak();
	/*
	 * The following code is equivalent to waddch(win, '\n')
	 * except that it does not do a wclrtoeol.
	 */
	if (doecho) {
		win->_curx = 0;
		if (win->_cury + 1 > win->_bmarg) {
			if (win->_scroll && !(win->_flags&_ISPAD)) {
				wrefresh(win);
				win->_need_idl = TRUE;
				_tscroll(win, 0);
				win->_cury++;
			}
		} else
			win->_cury++;
	}
	if (! (win->_flags&_ISPAD))
		wrefresh(win);
	if (ch == ERR)
		return ERR;
	return OK;
}
