/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_touch.c	1.2"
/*
 * Force the soft-labels to appear changed.
 */
#include "curses.ext"

slk_touch()
{
    register struct slkdata *SLK = SP->slk;
    register int i;

    if (SLK)
	{
	SLK->fl_changed = TRUE;
	for (i=0; i<8; i++)
	    {
	    SLK->changed[i] = TRUE;
	    SLK->scrlabel[i][0] = '\0';
	    }
	}
}
