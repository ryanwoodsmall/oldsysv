/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/winch.c	1.2"
#include	"curses_inc.h"

chtype
winch(win)
register	WINDOW	*win;
{
    return (win->_y[win->_cury][win->_curx]);
}
