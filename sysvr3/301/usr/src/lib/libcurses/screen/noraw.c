/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/noraw.c	1.4"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

noraw()
{
	_noraw();
	if (SP)
		SP->fl_rawmode=FALSE;
	reset_prog_mode();
#ifdef FIONREAD
	cur_term->timeout = 0;
#endif /* FIONREAD */
}

_noraw()
{
#ifdef SYSV
	/* Enable interrupt characters */
	PROGTTY.c_lflag |= ISIG;
	/* PROGTTY.c_cc[VINTR] = SHELLTTY.c_cc[VINTR]; */
	/* PROGTTY.c_cc[VQUIT] = SHELLTTY.c_cc[VQUIT]; */

	/* Disallow 8 bit input/output */
	PROGTTY.c_iflag |= ISTRIP;
	PROGTTY.c_iflag |= IXON;
	PROGTTY.c_cflag &= ~CSIZE;
	PROGTTY.c_cflag |= CS7;
	PROGTTY.c_cflag |= PARENB;
	_nocbreak();
#else
	PROGTTY.sg_flags &= ~RAW;
# ifdef DEBUG
	if(outf) fprintf(outf, "_noraw(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif
#endif
}
