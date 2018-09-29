/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/offscreen.c	1.1"

#include	<curses.h>
#include	<term.h>
#include	"wish.h"

unsigned	VT_firstline;
unsigned	VT_lastline;

off_screen(srow, scol, rows, cols)
unsigned	srow;
unsigned	scol;
unsigned	rows;
unsigned	cols;
{
	return srow >= VT_lastline || scol >= columns || srow + rows > VT_lastline - VT_firstline || scol + cols > columns;
}
