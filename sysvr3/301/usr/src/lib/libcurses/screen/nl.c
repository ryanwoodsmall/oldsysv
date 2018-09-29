/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/nl.c	1.1.1.1"
/*
 * Routines to deal with setting and resetting modes in the tty driver.
 * See also setupterm.c in the termlib part.
 */
#include "curses.ext"

nl()
{
#ifdef SYSV
/*	PROGTTY.c_iflag |= ICRNL;	this messes up input virt. */
	PROGTTY.c_oflag |= ONLCR;
# ifdef DEBUG
	if(outf) fprintf(outf, "nl(), file %x, SP %x, flags %x,%x\n", SP->term_file, SP, PROGTTY.c_iflag, PROGTTY.c_oflag);
# endif
#else
	/* this messes up input virt. as well, but can't be helped. */
	PROGTTY.sg_flags |= CRMOD;
# ifdef DEBUG
	if(outf) fprintf(outf, "nl(), file %x, SP %x, flags %x\n", SP->term_file, SP, PROGTTY.sg_flags);
# endif
#endif
	reset_prog_mode();
}
