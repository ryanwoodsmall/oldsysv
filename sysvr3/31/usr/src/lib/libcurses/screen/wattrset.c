/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wattrset.c	1.7"
#include	"curses_inc.h"

wattrset(win,a)
WINDOW	*win;
chtype	a;
{
    win->_attrs = (a | win->_bkgd) & A_ATTRIBUTES;
    return (1);
}
