/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/makenew.c	1.2"
# include	"curses.ext"

/*
 *	This routine sets up a window buffer and returns a pointer to it.
 */
WINDOW *
_makenew(num_lines, num_cols, begy, begx)
int	num_lines, num_cols, begy, begx;
{
	register int	i;
	register WINDOW	*win;
	register int	by, bx, nlines, ncols;
	extern char *malloc(), *calloc();

	by = begy;
	bx = begx;
	nlines = num_lines;
	ncols = num_cols;

	if (nlines <= 0 || ncols <= 0 || by > lines || bx > COLS)
		return (WINDOW *) NULL;

# ifdef	DEBUG
	if(outf) fprintf(outf, "MAKENEW(%d, %d, %d, %d)\n", nlines, ncols, by, bx);
# endif
	/* use calloc because everything needs to be zero */
	if ((win = (WINDOW *) calloc(1, sizeof (WINDOW))) == NULL) {
		(void) fprintf (stderr, "calloc returned NULL in _makenew\n");
		return (WINDOW *) NULL;
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "MAKENEW: nlines = %d\n", nlines);
# endif
	if ((win->_y = (chtype **) malloc(nlines * sizeof (chtype *))) == NULL) {
		(void) fprintf (stderr, "malloc returned NULL in _makenew\n");
		goto out_win;
	}
	/* use calloc because everything needs to be zero */
	if ((win->_firstch = (short *) calloc((unsigned)nlines, sizeof (short))) == NULL) {
		(void) fprintf (stderr, "calloc returned NULL in _makenew\n");
		goto out__y;
	}
	if ((win->_lastch = (short *) malloc(nlines * sizeof (short))) == NULL) {
		(void) fprintf (stderr, "malloc returned NULL in _makenew\n");
	out__y:	free((char *)win->_y);
	out_win:free((char *)win);
		free((char *)win);
		return (WINDOW *) NULL;
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "MAKENEW: ncols = %d\n", ncols);
# endif
	win->_cury = win->_curx = 0;
	win->_clear = (nlines == lines && ncols == COLS);
	win->_maxy = nlines;
	win->_maxx = ncols;
	win->_begy = by;
	win->_begx = bx;
	win->_scroll = win->_leave = FALSE;
	win->_use_idl = 1;
	win->_need_idl = 0;
	win->_tmarg = 0;
	win->_bmarg = nlines - 1;
	win->_flags |= _WINCHANGED;
	{
		register short *lastch = win->_lastch;
		register int ncols1 = ncols - 1;
		for (i = 0; i < nlines; i++)
			lastch[i] = ncols1;
	}
	if (bx + ncols == COLS) {
		win->_flags |= _ENDLINE;
		/* Full window: scrolling heuristics (linefeed) work */
		if (nlines == lines && ncols == COLS &&
		    by == 0 && bx == 0 && scroll_forward)
			win->_flags |= _FULLWIN;
		/* Scrolling window: it might scroll on us by accident */
		if (by + nlines == lines && auto_right_margin &&
		        (!cursor_left ||
			 (!enter_insert_mode && !insert_character)))
			win->_flags |= _SCROLLWIN;
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "MAKENEW: win->_clear = %d\n", win->_clear);
	if(outf) fprintf(outf, "MAKENEW: win->_leave = %d\n", win->_leave);
	if(outf) fprintf(outf, "MAKENEW: win->_scroll = %d\n", win->_scroll);
	if(outf) fprintf(outf, "MAKENEW: win->_flags = %0.2o\n", win->_flags);
	if(outf) fprintf(outf, "MAKENEW: win->_maxy = %d\n", win->_maxy);
	if(outf) fprintf(outf, "MAKENEW: win->_maxx = %d\n", win->_maxx);
	if(outf) fprintf(outf, "MAKENEW: win->_begy = %d\n", win->_begy);
	if(outf) fprintf(outf, "MAKENEW: win->_begx = %d\n", win->_begx);
# endif
	return win;
}
