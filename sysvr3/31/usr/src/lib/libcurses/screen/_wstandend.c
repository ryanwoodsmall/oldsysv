/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_wstandend.c	1.2"

#define		NOMACROS
#include	"curses_inc.h"

wstandend(win)
WINDOW	*win;
{
    wattrset(win, A_NORMAL);
    return (1);
}
