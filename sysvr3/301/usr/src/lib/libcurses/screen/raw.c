/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/raw.c	1.5"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

raw()
{
#ifdef DEBUG
# ifdef SYSV
	if(outf) fprintf(outf, "raw(), file %x, SP %x, iflag %x, cflag %x\n",
		SP->term_file, SP,
		PROGTTY.c_iflag, PROGTTY.c_cflag);
# else
	if(outf) fprintf(outf, "raw(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif /* SYSV */
#endif
#ifdef SYSV
	/* Disable interrupt characters */
	PROGTTY.c_lflag &= ~ISIG;
	/* PROGTTY.c_cc[VINTR] = 0377; */
	/* PROGTTY.c_cc[VQUIT] = 0377; */

	/* Allow 8 bit input/output */
	PROGTTY.c_iflag &= ~ISTRIP;
	PROGTTY.c_iflag &= ~IXON;
	PROGTTY.c_cflag &= ~CSIZE;
	PROGTTY.c_cflag |= CS8;
	PROGTTY.c_cflag &= ~PARENB;
	_cbreak();
#else
	PROGTTY.sg_flags |= RAW;
	_nocbreak();
#endif
	xon_xoff = 0;	/* Can't use xon/xoff in raw mode */
	if (SP)
		SP->fl_rawmode=TRUE;
	reset_prog_mode();
#ifdef FIONREAD
	cur_term->timeout = 0;
#endif /* FIONREAD */
}
