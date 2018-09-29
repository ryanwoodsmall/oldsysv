/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/newpad.c	1.1.1.1"
# include	"curses.ext"

/*
 * Like newwin, but makes a pad instead of a window.  A pad is not
 * associated with part of the screen, so it can be bigger.
 */
WINDOW *
newpad(nlines, ncols)
register int	nlines;
{
	register WINDOW	*pad;
	register int i;
	char *malloc();
	extern WINDOW *_makenew();
	extern void memSset();

	if ((pad = _makenew(nlines, ncols, 0, 0)) == NULL)
		return (WINDOW *) NULL;
	pad->_flags |= _ISPAD;
	pad->_pminrow = -1; pad->_pmincol = -1;
	pad->_sminrow = -1; pad->_smincol = -1;
	pad->_smaxrow = -1; pad->_smaxcol = -1;
	for (i = 0; i < nlines; i++)
		if ((pad->_y[i] = (chtype *) malloc(ncols * sizeof (chtype))) == NULL) {
			register int j;
			(void) fprintf (stderr, "malloc returned NULL in newpad\n");

			for (j = 0; j < i; j++)
				free((char *)pad->_y[j]);
			free((char *)pad->_firstch);
			free((char *)pad->_lastch);
			free((char *)pad->_y);
			free((char *)pad);
			return (WINDOW *) NULL;
		}
		else
			memSset(pad->_y[i], (chtype) ' ', ncols);
	pad->_yoffset = SP->Yabove;
	return pad;
}
