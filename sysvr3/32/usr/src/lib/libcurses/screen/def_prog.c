/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/def_prog.c	1.3"
#include "curses_inc.h"

def_prog_mode()
{
    /* ioctl errors are ignored so pipes work */
#ifdef SYSV
    (void) ioctl(cur_term -> Filedes, TCGETA, &(PROGTTY));
#else
    (void) ioctl(cur_term -> Filedes, TIOCGETP, &(PROGTTY));
#endif
    return (OK);
}
