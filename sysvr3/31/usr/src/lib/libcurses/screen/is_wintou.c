/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/is_wintou.c	1.1"
#include	"curses_inc.h"

is_wintouched(win)
WINDOW	*win;
{
    return (win->_flags & _WINCHANGED);
}
