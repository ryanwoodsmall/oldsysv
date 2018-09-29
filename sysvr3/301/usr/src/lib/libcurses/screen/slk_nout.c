/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_nout.c	1.1"
/*
 * This routine acts like wnoutrefresh in that it arranges to
 * output the slk's the next time doupdate() is called.
 */

#include "curses.ext"

slk_noutrefresh()
{
    register struct slkdata *SLK = SP->slk;
    register int i;
    int save_y, save_x;
    
    if (SLK && SLK->window)
	{
        for (i = 0; i < 8; i++)
	    SLK->changed[i] = FALSE;
	if (SLK->fl_changed)
	    {
	    getsyx(save_y,save_x);
	    wnoutrefresh(SLK->window);
	    setsyx(save_y,save_x);
	    }
	SLK->fl_changed = FALSE;
	}
}
