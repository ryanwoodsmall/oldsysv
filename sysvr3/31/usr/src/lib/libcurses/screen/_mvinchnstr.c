/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvinchnstr.c	1.1"

#define	NOMACROS

#include	"curses_inc.h"

mvinchnstr(y, x, s, n)
int	y, x, n;
chtype	*s;
{
    return (mvwinchnstr(stdscr, y, x, s, n));
}
