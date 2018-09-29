/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvwinnstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvwinnstr(win, y, x, s, n)
WINDOW *win;
int	y, x, n;
char	*s;
{
    return (wmove(win, y, x)==ERR?ERR:winnstr(win, s, n));
}
