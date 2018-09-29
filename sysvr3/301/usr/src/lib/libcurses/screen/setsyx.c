/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setsyx.c	1.2"
/*
	Set the current screen coordinates (y,x).

	This routine may be called before doupdate(). It tells doupdate()
	where to leave the cursor instead of the location of (x,y) of the
	last window that was wnoutrefreshed or pnoutrefreshed.
	If x and y are negative, then the cursor will be left wherever
	curses decides to leave it, as if leaveok() had been TRUE for the
	last window refreshed.
*/
#include "curses.ext"

setsyx (y, x)
register int y, x;
{
    if (y < 0 && x < 0)
	{
	SP->leave_leave = TRUE;
	SP->leave_x = SP->leave_y = 0;
	}
    else
	{
	SP->leave_leave = FALSE;
	SP->leave_x = x;
	SP->leave_y = y;
	}
}
