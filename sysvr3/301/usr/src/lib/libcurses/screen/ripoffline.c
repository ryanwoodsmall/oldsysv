/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/ripoffline.c	1.1"
/*
 * This routine is used by the slk initialization routines. It sets it up
 * such that a line is removed from the user's screen by initscr. This
 * function must be called BEFORE initscr. It works by leaving a cookie 
 * which tells initscr to reduce the size of stdscr by one for each line
 * ripped off. This routine has been generalized so that other applications
 * can make use of it in a straightforward manner.
 *
 *					Tony Hansen
 */

#include "curses.ext"

ripoffline (line, initfunction)
int line;
int (*initfunction)();
{
    if (_ripcounter < 5)
        {
	_ripstruct[_ripcounter].line = line;
	_ripstruct[_ripcounter++].initfunction = initfunction;
	}
}
