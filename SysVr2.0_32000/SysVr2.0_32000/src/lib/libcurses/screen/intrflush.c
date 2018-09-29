/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#include "curses.ext"
/*	@(#) intrflush.c: 1.1 10/15/83	(1.19	3/17/83)	*/

/*
 * TRUE => flush input when an interrupt key is pressed
 */
intrflush(win,bf)
WINDOW *win; int bf;
{
#ifdef USG
	if (bf)
		(cur_term->Nttyb).c_lflag &= ~NOFLSH;
	else
		(cur_term->Nttyb).c_lflag |= NOFLSH;
#else
	/* can't do this in 4.1BSD or V7 */
#endif
	reset_prog_mode();
}
