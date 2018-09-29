/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/curs_set.c	1.2"

#include "curses.ext"

/*
    Change the style of cursor in use. If a desired style is not available,
    at least negate the extreme opposite setting. That is, if cvvis is asked
    for, but not available, and invis is possible, at least output cnorm.
*/

int curs_set (visibility)
int visibility;
{
    extern int _outch();
    int ret, cur = 1;

    if (SP)
	ret = SP->cursorstate;
    else
	ret = 1;

    switch (visibility)
	{
	case 0:			/* off */
	    if (cursor_invisible)
		tputs (cursor_invisible, 1, _outch), cur = 0;
	    else if (cursor_visible && cursor_normal)
		tputs (cursor_normal, 1, _outch), cur = 1;
	    break;
	case 1:			/* on */
	    if (cursor_normal)
		tputs (cursor_normal, 1, _outch), cur = 1;
	    else if (cursor_invisible && cursor_visible)
		tputs (cursor_visible, 1, _outch), cur = 2;
	    break;
	case 2:			/* very visible */
	    if (cursor_visible)
		tputs (cursor_visible, 1, _outch), cur = 2;
	    else if (cursor_invisible && cursor_normal)
		tputs (cursor_normal, 1, _outch), cur = 1;
	    break;
	}
    if (SP)
	SP->cursorstate = cur;
    return ret;
}
