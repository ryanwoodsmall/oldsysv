/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/resetty.c	1.2"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

resetty()
{
#ifdef SYSV
	if (SP == NULL || (SP->save_tty_buf.c_cflag&CBAUD) == 0)
		return;	/* Never called savetty */
#else
	if (SP == NULL || SP->save_tty_buf.sg_ospeed == 0)
		return;	/* Never called savetty */
#endif
	PROGTTY = SP->save_tty_buf;
#ifdef DEBUG
# ifdef SYSV
	if(outf) fprintf(outf, "savetty(), file %x, SP %x, flags %x,%x,%x,%x\n", SP->term_file, SP, PROGTTY.c_iflag, PROGTTY.c_oflag, PROGTTY.c_cflag, PROGTTY.c_lflag);
# else
	if(outf) fprintf(outf, "resetty(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif
#endif
	reset_prog_mode();
}
