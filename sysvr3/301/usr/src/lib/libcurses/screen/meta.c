/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/meta.c	1.1.1.1"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

/*
 * TRUE => all 8 bits of input character should be passed through.
 */
meta(win,bf)
WINDOW *win; int bf;
{
	int _outch();

	if (!has_meta_key)
		return ERR;
	/*
	 * Do the appropriate fiddling with the tty driver to make it send
	 * all 8 bits through.  On SYSV this means clearing ISTRIP, on
	 * V7 you have to resort to RAW mode.
	 */
#ifdef SYSV
	if (bf) {
		PROGTTY.c_iflag &= ~ISTRIP;
		PROGTTY.c_cflag &= ~CSIZE;
		PROGTTY.c_cflag |= CS8;
		PROGTTY.c_cflag &= ~PARENB;
	} else {
		PROGTTY.c_iflag |= ISTRIP;
		PROGTTY.c_cflag &= ~CSIZE;
		PROGTTY.c_cflag |= CS7;
		PROGTTY.c_cflag |= PARENB;
	}
#else
	if (bf)
		raw();
	else
		noraw();
#endif
	reset_prog_mode();

	/*
	 * Do whatever is needed to put the terminal into meta-mode.
	 */
	if (bf)
		tputs(meta_on, 1, _outch);
	else
		tputs(meta_off, 1, _outch);

	/* Keep track internally. */
	win->_use_meta = bf;

	return OK;
}
