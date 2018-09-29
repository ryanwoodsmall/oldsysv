/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/reset_sh.c	1.2"
#include "curses.ext"

reset_shell_mode()
{
#ifdef DIOCSETT
	/*
	 * Restore any virtual terminal setting.  This must be done
	 * before the TIOCSETN because DIOCSETT will clobber flags like xtabs.
	 */
	cur_term -> old.st_flgs |= TM_SET;
	(void) ioctl(cur_term->Filedes, DIOCSETT, &cur_term -> old);
#endif
	if (BR(Ottyb)) {
		(void) ioctl(cur_term -> Filedes,
#ifdef SYSV
			TCSETAW,
#else
			TIOCSETN,
#endif
			&SHELLTTY);
# ifdef LTILDE
	if (cur_term -> newlmode != cur_term -> oldlmode)
		(void) ioctl(cur_term -> Filedes, TIOCLSET,
			&cur_term -> oldlmode);
# endif
	}
}
