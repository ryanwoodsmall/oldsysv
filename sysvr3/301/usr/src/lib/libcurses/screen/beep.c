/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/beep.c	1.5"

#include "curses.ext"

extern _outch();
extern int InputPending;

beep()
{
    int savexon = xon_xoff;
#ifdef DEBUG
	if(outf) fprintf(outf, "beep().\n");
#endif
    xon_xoff = 0;
    if (bell)
	tputs (bell, 0, _outch);
    else
	tputs (flash_screen, 0, _outch);
    xon_xoff = savexon;
    __cflush();
    if (InputPending)
	doupdate();
}

