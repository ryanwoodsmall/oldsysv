/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/touchwin.c	1.6"
#include	"curses_inc.h"

touchwin(win)
WINDOW	*win;
{
    return (wtouchln(win,0,win->_maxy,TRUE));
}
