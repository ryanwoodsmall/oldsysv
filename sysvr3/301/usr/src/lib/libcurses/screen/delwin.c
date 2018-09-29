/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/delwin.c	1.3"
# include	"curses.ext"

/*
 *	This routine deletes a _window and releases it back to the system.
 *
 */
delwin(win)
register WINDOW	*win;
{
	register int	i;

	if (!(win->_flags & _SUBWIN))
		for (i = 0; i < win->_maxy && win->_y[i]; i++)
			free((char *) win->_y[i]);
	free((char *) win->_y);
	free((char *) win->_firstch);
	free((char *) win->_lastch);
	free((char *) win);
}
