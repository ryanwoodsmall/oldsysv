/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/restart.c	1.5"

#include "curses.ext"

/*
 * This is useful after saving/restoring memory from a file (e.g. as
 * in a rogue save game).  It assumes that the modes and windows are
 * as wanted by the user, but the terminal type and baud rate may
 * have changed.
 */
restartterm(term, filenum, errret)
char *term;
int filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int *errret;
{
	int saveecho = SP->fl_echoit;
	int savecbreak = SP->fl_rawmode;
	int saveraw;
	int savenl;
	extern int _called_before;

#ifdef SYSV
	saveraw = !(PROGTTY.c_lflag & ISIG);
	savenl = PROGTTY.c_iflag & ONLCR;
#else
	saveraw = PROGTTY.sg_flags | RAW;
	savenl = PROGTTY.sg_flags & CRMOD;
#endif

	_called_before = 0;
	(void) setupterm(term, filenum, (int *) 0);

	/*
	 * Restore curses settable flags, leaving other stuff alone.
	 */
	if (saveecho)
		echo();
	else
		noecho();

	nocbreak();
	noraw();
	if (savecbreak)
		cbreak();
	else if (saveraw)
		raw();

	if (savenl)
		nl();
	else
		nonl();

	reset_prog_mode();

	LINES = lines - SP->Yabove - SP->Ybelow;
	COLS = columns;
}
