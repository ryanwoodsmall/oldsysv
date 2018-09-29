/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/pechochar.c	1.3"
/*
 *  These routines short-circuit much of the innards of curses in order to get
 *  a single character output to the screen quickly!
 *
 *  pechochar(WINDOW *pad, chtype ch) is functionally equivalent to
 *  waddch(WINDOW *pad, chtype ch), prefresh(WINDOW *pad, `the same arguments
 *  as in the last prefresh or pnoutrefresh')
 */

#include	"curses.ext"
extern int _ll_echochar();

int
pechochar (pad, ch)
register WINDOW *pad;
chtype ch;
{
	register int pcury = pad->_cury;
	register int pcurx = pad->_curx;
	register int SPy = pcury - pad->_pminrow + pad->_sminrow + pad->_yoffset;
	register int SPx = pcurx - pad->_pmincol + pad->_smincol;
	register chtype c = ch | pad->_attrs,
		rawc = ch & A_CHARTEXT,
		attrs = c & A_ATTRIBUTES;

	/* Check for reasons that force us to do a waddch(). */
	if ((rawc < ' ' || rawc >= 0177) ||	/* control char */
	    (pcurx >= pad->_maxx - 1) ||	/* right edge of window */
	    (SPy > 0 && SPy < lines &&		/* no line structure */
	    SP->cur_body [SPy+1] == NULL) ||
	    (pad->_flags&_WINCHANGED) ) {	/* no recent refresh() */
		(void) waddch (pad, (chtype) ch);
		pnoutrefresh (pad,
			pad->_pminrow, pad->_pmincol,
			pad->_sminrow, pad->_smincol,
			pad->_smaxrow, pad->_smaxcol);
		return doupdate ();
	}

	/* waddch(): Handle the virtual window. */
	pad->_y [pcury][pcurx] = c;
	pad->_curx++;

	/* pnoutrefresh(): Handle the virtual screen. */
	/* Check for reasons that force us to do a pnoutrefresh(). */
	if (pad->_clear || SP->doclear ||	/* screen has been cleared */
	    magic_cookie_glitch > 0 ||		/* too difficult for here */
	    SP->fl_changed ||			/* no recent doupdate() */
	    (SP->slk && SP->slk->fl_changed) ||
	    pcurx < pad->_pmincol ||		/* ch !w/in refreshed area */
	    pcurx > pad->_pmincol + pad->_smaxcol - pad->_smincol ||
	    pcury < pad->_pminrow ||
	    pcury > pad->_pminrow + pad->_smaxrow - pad->_sminrow) {
		pad->_firstch [pcury] = pad->_lastch [pcury] = pcurx;
		pad->_flags |= _WINCHANGED;
		pnoutrefresh (pad, pad->_pminrow, pad->_pmincol,
			pad->_sminrow, pad->_smincol,
			pad->_smaxrow, pad->_smaxcol);
		return doupdate ();
	}

	/* doupdate: Handle the physical screen. */

	return _ll_echochar(SPy, SPx, c, attrs, rawc);
}
