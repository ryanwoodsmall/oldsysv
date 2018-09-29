/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#) wstandout.c: 1.1 10/15/83	(1.2	4/7/82)	*/
/*
 * 1/26/81 (Berkeley) @(#)standout.c	1.1
 */

# include	"curses.ext"

/*
 * enter standout mode
 */
wstandout(win)
register WINDOW	*win;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "WSTANDOUT(%x)\n", win);
#endif

	win->_attrs |= A_STANDOUT;
	return 1;
}
