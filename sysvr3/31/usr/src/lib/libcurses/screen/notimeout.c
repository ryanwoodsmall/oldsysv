/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/notimeout.c	1.6"
#include	"curses_inc.h"

notimeout(win,bf)
WINDOW	*win;
int	bf;
{
    win->_notimeout = (bf) ? TRUE : FALSE;
    return (OK);
}
