/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/vfork.c	1.2"

#include	<curses.h>
#include	<term.h>
#include	"wish.h"

/* Functions for use before and after forking processes */

void
vt_before_fork()
{
	endwin();
}

void
vt_after_fork()
{
	/*
	 * Reset color pairs upon return from UNIX ....
	 * If this isn't a color terminal then set_def_colors()
	 * returns without doing anything
	 */
	set_def_colors();
}

void
fork_clrscr()
{
	putp(tparm(clear_screen));
	fflush(stdout);
}
