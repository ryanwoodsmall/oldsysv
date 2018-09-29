/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setnonl.c	1.2"
#include	"curses_inc.h"

_setnonl(bf)
int	bf;
{
    SP->fl_nonl = bf;
    return (OK);
}
