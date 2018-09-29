/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_rest.c	1.3"
/*
 * Undo a slk_clear().
 */

#include "curses.ext"

slk_restore()
{
    register struct slkdata *SLK = SP->slk;
    register WINDOW *window;
    register int i;
    int y, x;
    extern int _outch();

    if (SLK)
	if ((window = SLK->window) != (WINDOW *) NULL)
	    {
	    getsyx(y,x);
	    for (i=0; i<8; i++)
		{
		wmove (window, 0, SLK->offset[i]);
		waddstr (window, "        ");
		wmove(window, 0, SLK->offset[i]);
		waddstr(window, SLK->label[i]);
		strcpy(SLK->scrlabel[i], SLK->label[i]);
		}
	    wnoutrefresh (SLK->window);
	    setsyx(y,x);
	    doupdate();
	    }
	else
	    {
	    doupdate();
	    for (i=0; i<8; i++)
		{
		tputs(tparm(plab_norm, i+1, SLK->label[i]), 1, _outch);
		SLK->changed[i] = FALSE;
		strcpy(SLK->scrlabel[i], SLK->label[i]);
		}
	    tputs(label_on, 1, _outch);
	    __cflush();
	    }
}
