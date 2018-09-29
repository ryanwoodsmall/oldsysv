/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/untouchwin.c	1.3"
#include	"curses_inc.h"

untouchwin(win)
WINDOW	*win;
{
    return (wtouchln(win,0,win->_maxy,FALSE));
}
