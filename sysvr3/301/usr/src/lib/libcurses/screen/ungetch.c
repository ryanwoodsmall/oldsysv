/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/ungetch.c	1.3"
/*
 * Place a key back onto the beginning of curses' input queue.
 */

#include "curses.ext"

ungetch(ch)
register int ch;
{
    /* _ungetch() an encoded version of the char */
    /* so that flushinp() won't get rid of it. */
    _ungetch(-ch - 0100, 1);
    SP->ungotten++;
}
