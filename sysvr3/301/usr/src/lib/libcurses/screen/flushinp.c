/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/flushinp.c	1.4.1.1"

#include "curses.ext"

flushinp()
{
	register short *inputQ;
	register int i, j, ch;
#ifdef DEBUG
	if(outf) fprintf(outf, "flushinp(), file %x, SP %x\n", SP->term_file, SP);
#endif
#ifdef SYSV
	(void) ioctl(cur_term -> Filedes, TCFLSH, 0);
#else
	/* for insurance against someone using their own buffer: */
	(void) ioctl(cur_term -> Filedes, TIOCGETP, &(PROGTTY));

	/*
	 * SETP waits on output and flushes input as side effect.
	 * Really want an ioctl like TCFLSH but Berkeley doesn't have one.
	 */
	(void) ioctl(cur_term -> Filedes, TIOCSETP, &(PROGTTY));
#endif
	/*
	 * Get rid of any typeahead which was read().
	 * Leave characters which were ungetch()'d.
	 */
	inputQ = SP->input_queue;
	for (i = 0, j = 0; j < INP_QSIZE; j++) {
		ch = inputQ[j];
		if (ch < 0) {
			inputQ[i++] = ch;
			if (ch == -1)
				break;
		}
	}

	/*
	 * Have to doupdate() because, if we've stopped output due to
	 * typeahead, now that typeahead is gone, so we'd better catch up.
	 */
	if (inputQ[0] == -1)
		doupdate();
}
