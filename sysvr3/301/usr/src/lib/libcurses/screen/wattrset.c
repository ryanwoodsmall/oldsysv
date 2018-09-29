/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wattrset.c	1.3"
/*
 * routines dealing with entering and exiting standout mode
 *
 */

# include	"curses.ext"

/*
 * Set selected attributes.
 */
wattrset(win, attrs)
register WINDOW	*win;
chtype attrs;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "WATTRON(%x, %o)\n", win, attrs);
#endif

	win->_attrs = attrs;
	return 1;
}
