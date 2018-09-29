/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getmaxyx.c	1.3"
#include	"curses_inc.h"

getmaxy(win)
WINDOW	*win;
{
    return (win->_maxy);
}

getmaxx(win)
WINDOW	*win;
{
    return (win->_maxx);
}
