/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/termerr.c	1.3"

#include 	"curses_inc.h"

char	*term_err_strings[] =
{
    "",
    "/usr/lib/terminfo is unaccessible",
    "I don't know anything about your \"%s\" terminal",
    "corrupted terminfo entry",
    "terminfo entry too long",
    "TERMINFO pathname for device exceeds 512 characters",
#ifdef	DEBUG
    "malloc returned NULL in function \"%s\"",
#else	/* DEBUG */
    "malloc returned NULL",
#endif	/* DEBUG */
    "terminfo file for \"%s\" terminal is not readable",
};

void
termerr()
{
    (void) fprintf(stderr, "Sorry, ");
    (void) fprintf(stderr, term_err_strings[term_errno], term_parm_err);
    (void) fprintf(stderr, ".\r\n");
}
