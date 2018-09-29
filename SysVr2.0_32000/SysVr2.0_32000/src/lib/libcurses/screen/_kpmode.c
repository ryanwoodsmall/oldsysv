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
/*	@(#) _kpmode.c: 1.1 10/15/83	(1.21	2/11/83)	*/

#include "curses.ext"

extern	int	_outch();

_kpmode(m)
{
#ifdef DEBUG
	if (outf) fprintf(outf, "kpmode(%d), SP->kp_state %d\n",
	m, SP->kp_state);
#endif
	if (m == SP->kp_state)
		return;
	if (m)
		tputs(keypad_xmit, 1, _outch);
	else
		tputs(keypad_local, 1, _outch);
	SP->kp_state = m;
}
