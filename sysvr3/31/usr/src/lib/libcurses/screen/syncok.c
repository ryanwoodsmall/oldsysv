/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/syncok.c	1.3"
#include	"curses_inc.h"

syncok(win,bf)
WINDOW	*win;
int	bf;
{
   return (win->_parent ? win->_sync = bf : ERR);
}
