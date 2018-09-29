/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/scrollok.c	1.7"
#include	"curses_inc.h"

scrollok(win,bf)
WINDOW	*win;
int	bf;
{
    win->_scroll = (bf) ? TRUE : FALSE;
    return (OK);
}
