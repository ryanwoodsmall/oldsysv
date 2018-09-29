/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_vline.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

vline(vertch, num_chars)
chtype	vertch;
int	num_chars;
{
    return (wvline(stdscr, vertch, num_chars));
}
