/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvwaddstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvwaddstr(win, y, x, str)
WINDOW	*win;
int	y, x;
char	*str;
{
    return (wmove(win, y, x)==ERR?ERR:waddstr(win, str));
}
