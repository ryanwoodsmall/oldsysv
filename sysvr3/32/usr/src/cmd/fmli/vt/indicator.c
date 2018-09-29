/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/indicator.c	1.1"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"

void
indicator(message, col)
char *message;
int col;
{
	WINDOW		*win;

	win = VT_array[ STATUS_WIN ].win;
	/* error check */
	mvwaddstr(win, 0, col, message);
	wnoutrefresh( win );
	doupdate();
}
