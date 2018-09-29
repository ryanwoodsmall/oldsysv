/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvhline.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvhline(y, x, c, n)
int	y, x, n;
chtype	c;
{
    return (mvwhline(stdscr, y, x, c, n));
}
