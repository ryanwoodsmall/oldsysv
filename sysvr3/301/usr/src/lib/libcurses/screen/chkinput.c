/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/chkinput.c	1.6.1.1"
/*
 * chk_input()
 *
 * Routine to check to see if any input is waiting.
 * It returns a 1 if there is input waiting, and a zero if
 * no input is waiting.
 *
 * This function replaces system calls to ioctl() with the FIONREAD
 * parameter. It enables curses to stop a screen refresh whenever
 * a character is input.
 * Standard BTL UNIX 4.0 or 5.0 does not handle FIONREAD.
 * Changes have also been made to 'll_refresh.c' and 'insdelline.c' to
 * call this routine as "InputPending = chk_input()".
 * (delay.c and getch.c also use FIONREAD for nodelay, select and fast peek,
 * but these routines have not been changed).
 *
 * Philip N. Crusius - July 20, 1983
 * Modified to handle various systems March 9, 1984 by Mark Horton.
 */
#include "curses.ext"

#ifdef FIONREAD
#define HAVE_CHK
int
_chk_input()
{
	int i;
	ioctl(SP->check_fd, FIONREAD, &i);
	return i > 0;
}
#endif /* FIONREAD */

#ifdef SELECT
# ifndef HAVE_CHK
# define HAVE_CHK
int
_chk_input()
{
	struct _timeval {
		long tv_sec;
		long tv_usec;
	};
	int ifds, ofds, efds, n;
	struct _timeval tv;

	ifds = 1 << SP->check_fd;
	ofds = efds = 0;
	tv.tv_sec = tv.t_usec = 0;
	n = select(20, &ifds, &ofds, &efds, &tv);
	return n > 0;
}
# endif /* HAVE_CHK */
#endif /* SELECT */

#ifndef HAVE_CHK
# ifdef SYSV

int
_chk_input()
{
	register int	fd;	/* input file descriptor index */
	char	c;		/* character input */

	/* If input is waiting in curses queue,	return(1).	*/
	if (SP->input_queue[0] != -1)
		return(1);

	/* Only check typeahead if the user is using our input */
	/* routines. This is because the ungetch() below will put */
	/* stuff into the inputQ that will never be read and the */
	/* screen will never get updated from now on. */
	/* This code should GO AWAY when a poll() or FIONREAD can */
	/* be done on the file descriptor as then the check */
	/* will be non-destructive. */
	if (!SP->fl_typeahdok)
		return 0;

	/*
	 * We won't be called unless fd is valid.
	 */
	fd = SP->check_fd;

	if (read(fd, &c, 1) > 0) {
		/*
		 * A character was waiting.  Put it at the end
		 * of the curses queue and return 1 to show that
		 * input is waiting.
		 */
		_ungetch(c, 0);
		return(1);
	} else { /* No input was waiting so return 0. */
		return(0);
	}
}
# else
int
_chk_input()
{
	return 0;
}
# endif /* SYSV */
#endif /* HAVE_CHK */

/*
    Place a char onto the beginning or end of the input queue.
*/
_ungetch(ch, beginning)
int ch, beginning;
{
	register i, temp;
	register short *inputQ = SP->input_queue;

	if (beginning)
		/* place the character at the beg of the Q */
		for (i = 0; i < INP_QSIZE; i++) {
			temp = inputQ[i];
			inputQ[i] = ch;
			if (ch == -1)
				break;
			ch = temp;
		}
	else
		/* place the character at the end of the Q */
		for (i = 0; i < INP_QSIZE; i++) {
			if (inputQ[i] == -1) {
				inputQ[i] = ch;
				if (i < INP_QSIZE - 1)
					inputQ[i+1] = -1;
				break;
			}
		}
}
