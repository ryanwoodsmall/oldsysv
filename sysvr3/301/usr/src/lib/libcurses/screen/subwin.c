/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/subwin.c	1.2"
# include	"curses.ext"

WINDOW *
subwin(orig, num_lines, num_cols, begy, begx)
register WINDOW	*orig;
int	num_lines, num_cols, begy, begx;
{

	register int i;
	register WINDOW	*win;
	register int by, bx, nlines, ncols;
	register int j, k;
	WINDOW *_makenew();

	by = begy;			/* move these into registers */
	bx = begx;
	nlines = num_lines;
	ncols = num_cols;

	/*
	 * make sure window fits inside the original one
	 */
# ifdef	DEBUG
	if(outf) fprintf(outf, "SUBWIN(%0.2o, %d, %d, %d, %d)\n", orig, nlines, ncols, by, bx);
# endif
	if (by < orig->_begy || bx < orig->_begx
	    || by + nlines > orig->_begy + orig->_maxy
	    || bx + ncols  > orig->_begx + orig->_maxx)
		return NULL;
	if (nlines == 0)
		nlines = orig->_maxy - orig->_begy - by;
	if (ncols == 0)
		ncols = orig->_maxx - orig->_begx - bx;
	if ((win = _makenew(nlines, ncols, by, bx)) == NULL)
		return NULL;
	win->_flags |= _SUBWIN;
	win->_yoffset = orig->_yoffset;
	j = by - orig->_begy;
	k = bx - orig->_begx;
	for (i = 0; i < nlines; i++)
		win->_y[i] = &orig->_y[j++][k];
	return win;
}
