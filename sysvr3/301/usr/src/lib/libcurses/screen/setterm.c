/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setterm.c	1.1"
# include	"curses.ext"

/*
 * Low level interface, for compatibility with old curses.
 */
setterm(type)
char *type;
{
	setupterm(type, 1, (int *) 0);
}
