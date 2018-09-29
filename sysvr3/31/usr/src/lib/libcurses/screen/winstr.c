/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/winstr.c	1.3"
#include	"curses_inc.h"

winstr(win,str)
WINDOW	*win;
char	*str;
{
    return (winnstr(win,str,win->_maxx - win->_curx));
}
