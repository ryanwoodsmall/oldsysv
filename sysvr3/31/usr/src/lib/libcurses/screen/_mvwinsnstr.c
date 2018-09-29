/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvwinsnstr.c	1.1"

#define	NOMACROS

#include	"curses_inc.h"

mvwinsnstr(win, y, x, s, n)
WINDOW	*win;
int	y, x;
char	*s;
int	n;
{
    return (wmove(win, y, x)==ERR?ERR:winsnstr(win, s, n));
}
