/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/cbreak.c	1.3.1.1"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

cbreak()
{
	_noraw();
	_cbreak();
	if (SP)
		SP->fl_rawmode = TRUE;
	reset_prog_mode();
#ifdef FIONREAD
	cur_term->timeout = 0;
#endif /* FIONREAD */
}

_cbreak()
{
#ifdef SYSV
	PROGTTY.c_iflag &= ~ICRNL;
	PROGTTY.c_lflag &= ~ICANON;
	PROGTTY.c_cc[VMIN] = 1;
	PROGTTY.c_cc[VTIME] = 0;
# ifdef DEBUG
	if(outf) fprintf(outf, "cbreak(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.c_lflag);
# endif
#else
	PROGTTY.sg_flags |= CBREAK;
# ifdef DEBUG
	if(outf) fprintf(outf, "cbreak(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif
#endif
}
