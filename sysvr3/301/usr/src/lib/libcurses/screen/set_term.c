/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/set_term.c	1.6"
#include "curses.ext"

SCREEN *
set_term(new)
SCREEN *new;
{
	register SCREEN *rv;

	if (new == SP)
		return SP;

	rv = SP;
#ifdef DEBUG
	if(outf) fprintf(outf, "setterm: old %x, new %x\n", rv, new);
#endif

#ifndef		NONSTANDARD
	SP = new;
#endif		/* NONSTANDARD */

	(void) set_curterm(SP->tcap);
	LINES = lines - SP->Yabove - SP->Ybelow;
	COLS = columns;
	stdscr = SP->std_scr;
	curscr = SP->cur_scr;
	return rv;
}
