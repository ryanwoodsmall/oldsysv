/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/leaveok.c	1.5"
#include	"curses_inc.h"

leaveok(win,bf)
WINDOW	*win;
int	bf;
{
    win->_leave = bf;
    return (OK);
}
