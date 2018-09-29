/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvinsch.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvinsch(y, x, c)
int	y, x;
chtype	c;
{
    return (wmove(stdscr, y, x)==ERR?ERR:winsch(stdscr, c));
}

