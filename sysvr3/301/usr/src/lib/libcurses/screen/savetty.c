/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/savetty.c	1.2"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

savetty()
{
	SP->save_tty_buf = PROGTTY;
#ifdef DEBUG
# ifdef SYSV
	if(outf) fprintf(outf, "savetty(), file %x, SP %x, flags %x,%x,%x,%x\n", SP->term_file, SP, PROGTTY.c_iflag, PROGTTY.c_oflag, PROGTTY.c_cflag, PROGTTY.c_lflag);
# else
	if(outf) fprintf(outf, "savetty(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif
#endif
}
