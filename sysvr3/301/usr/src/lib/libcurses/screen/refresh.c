/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/refresh.c	1.3"
/*
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

# include	"curses.ext"

/* Put out window and update screen */
wrefresh(win)
WINDOW	*win;
{
	wnoutrefresh(win);
	return doupdate();
}
