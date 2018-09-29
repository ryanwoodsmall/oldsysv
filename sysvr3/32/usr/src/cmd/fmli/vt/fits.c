/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/fits.c	1.1"

#include	<curses.h>
#include	<term.h>
#include	"wish.h"
#include	"vtdefs.h"

unsigned	VT_firstline;
unsigned	VT_lastline;

/*
 * determines whether or not a window this big will fit with a border
 * around it
 */
bool
fits(flags, rows, cols)
unsigned	flags;
unsigned	rows;
unsigned	cols;
{
	if (!(flags & VT_NOBORDER)) {
		rows += 2;
		cols += 2;
	}
	return rows <= VT_lastline - VT_firstline && cols <= columns;
}
