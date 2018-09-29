/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getattrs.c	1.2"
#include	"curses_inc.h"

chtype
getattrs(win)
WINDOW	*win;
{
    return (win->_attrs);
}
