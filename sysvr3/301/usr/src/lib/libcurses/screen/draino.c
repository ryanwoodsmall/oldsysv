/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/draino.c	1.3.1.1"

#include "curses.ext"

/*
 * Code for various kinds of delays.  Most of this is nonportable and
 * requires various enhancements to the operating system, so it won't
 * work on all systems.  It is included in curses to provide a portable
 * interface, and so curses itself can use it for function keys.
 */

#define NAPINTERVAL 100
/*
 * Wait until the output has drained enough that it will only take
 * ms more milliseconds to drain completely.
 * Needs Berkeley TIOCOUTQ ioctl.  Returns ERR if impossible.
 */
int
draino(ms)
int ms;
{
#ifdef TIOCOUTQ
# define _DRAINO
	int ncneeded;	/* number of chars = that many ms */

	/* 10 bits/char, 1000 ms/sec, baudrate in bits/sec */
	ncneeded = baudrate() * ms / (10 * 1000);
	for (;;) {
		int rv;		/* ioctl return value */
		int ncthere = 0;/* number of chars actually in output queue */
		rv = ioctl(cur_term->Filedes, TIOCOUTQ, &ncthere);
#ifdef DEBUG
		if (outf) fprintf(outf, "draino: rv %d, ncneeded %d, ncthere %d\n",
			rv, ncneeded, ncthere);
#endif
		if (rv < 0)
			return ERR;	/* ioctl didn't work */
		if (ncthere <= ncneeded) {
			return OK;
		}
		napms(NAPINTERVAL);
	}
#endif

#ifdef TCSETAW
# define _DRAINO
	/*
	 * SYSV simulation - waits until the entire queue is empty,
	 * then sets the state to what it already is (e.g. no-op).
	 * Unfortunately this only works if ms is zero.
	 */
	if (ms <= 0) {
		(void) ioctl(cur_term->Filedes, TCSETAW, &PROGTTY);
		return OK;
	} else
		return ERR;
#endif

#ifndef _DRAINO
	/* No way to fake it, so we return failure. */
	/* Used #else to avoid warning from compiler about unreached stmt */
	return ERR;
#endif
}
