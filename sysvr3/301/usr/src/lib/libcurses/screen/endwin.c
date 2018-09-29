/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/endwin.c	1.7.1.1"
/*
 * Clean things up before exiting
 *
 * ??? (Berkeley) ??? (from 4.1BSD)
 */

# include	"curses.ext"

extern int _outch();

#ifdef _VR2_COMPAT_CODE
int _endwin = 0;
#endif

isendwin()
{
	return SP->fl_endwin;
}
endwin()
{
	if (SP->fl_endwin)
		return;

	/* Flush out any output not output due to typeahead. */
	_force_doupdate();

	/* Close things down. */
	_c_clean();
	SP->fl_endwin = TRUE;

#ifdef _VR2_COMPAT_CODE
	_endwin = TRUE;
#endif

	SP->doclear = TRUE;
	reset_shell_mode();
	__cflush();
}

_force_doupdate()
{
	short savecf, saveci, saveiQ;

	/* Save the current values in these places. */
	saveci = SP->check_input;
	savecf = SP->check_fd;
	saveiQ = SP->input_queue[0];

	/* Stuff some values in to turn off typeahead checking. */
	/* Do it this way because calling typeahead() is slow. */
	SP->check_fd = -1;
	SP->input_queue[0] = -1;
	SP->check_input = 9999;
	doupdate();

	/* Put things back in case of another initscr. */
	SP->check_input = saveci;
	SP->check_fd = savecf;
	SP->input_queue[0] = saveiQ;
}

static _c_clean ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_c_clean().\n");
#endif
	_fixdelay((bool)SP->fl_nodelay, (bool)FALSE);
	if (stdscr->_use_meta)
		tputs(meta_off, 1, _outch);
	_pos(lines-1, 0);

	_hlmode (0);
	_kpmode(0);
	SP->virt_irm = 0;
	_window(0, lines-1, 0, columns-1);
	_syncmodes();
	if (SP->cursorstate != 1) {
		tputs(cursor_normal, 0, _outch);
		SP->cursorstate = 1;
	}
	tputs(exit_ca_mode, 0, _outch);
}
