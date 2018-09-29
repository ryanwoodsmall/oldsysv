/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_clear.c	1.3"

#include "curses.ext"

/*
 * Erase the soft key labels from the screen but do not change their values.
 * This would be used by an exec routine which moves to the lower left
 * first and wants the line cleared. Since the labels may be there, this
 * routine is provided to clear those out.
 */

slk_clear()
{
    register struct slkdata *SLK = SP->slk;
    register int i;
    register WINDOW *window;
    int x, y;
    extern int _outch();
    char *blanks = "        ";

    if (SLK)
	if ((window = SLK->window) != (WINDOW *) NULL)
	    {
	    getsyx(y,x);
	    werase (window);
	    wnoutrefresh (window);
	    setsyx(y,x);
	    doupdate();
	    for (i=0; i<8; i++)
		SLK->scrlabel[i][0] = '\0';
	    }
	else
	    {
	    doupdate();
	    for (i=0; i<8; i++)
		{
		tputs(tparm(plab_norm, i+1, blanks), 1, _outch);
		SLK->scrlabel[i][0] = '\0';
		SLK->changed[i] = FALSE;
		}
	    tputs(label_on, 1, _outch);
	    __cflush();
	    }
}
