/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setecho.c	1.4"
#include "curses_inc.h"

_setecho(bf)
int	bf;
{
    SP->fl_echoit = bf;
    return (OK);
}
