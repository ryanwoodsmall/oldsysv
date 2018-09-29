/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/idcok.c	1.1"
#include	"curses_inc.h"

void
idcok(win, bf)
WINDOW	*win;
int	bf;
{
    win->_use_idc = (bf) ? TRUE : FALSE;
}
