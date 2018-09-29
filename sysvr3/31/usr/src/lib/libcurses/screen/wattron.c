/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wattron.c	1.7"
#include	"curses_inc.h"

wattron(win,a)
WINDOW	*win;
chtype	a;
{
    win->_attrs |= (a & A_ATTRIBUTES);
    return (1);
}
