/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_intrflush.c	1.2"

#define		NOMACROS
#include	"curses_inc.h"

intrflush(win, flag)
WINDOW	*win;
int	flag;
{
    _setqiflush(flag);
    return (OK);
}
