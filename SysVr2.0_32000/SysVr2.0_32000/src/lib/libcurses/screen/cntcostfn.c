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
/*	@(#) cntcostfn.c: 1.1 10/15/83	(1.13	8/20/82)	*/

static counter = 0;
/* ARGSUSED */
_countchar(ch)
char ch;
{
	counter++;
}

/*
 * Figure out the _cost in characters to print this string.
 * Due to padding, we can't just use strlen, so instead we
 * feed it through tputs and trap the results.
 * Even if the terminal uses xon/xoff handshaking, count the
 * pad chars here since they estimate the real time to do the
 * operation, useful in calculating costs.
 */
_cost_fn(str, affcnt)
char *str;
{
	int save_xflag = xon_xoff;

	if (str == NULL)
		return INFINITY;
	counter = 0;
	xon_xoff = 0;
	tputs(str, affcnt, _countchar);
	xon_xoff = save_xflag;
	return counter;
}
