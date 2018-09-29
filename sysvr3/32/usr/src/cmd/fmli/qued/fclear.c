/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/fclear.c	1.1"

#include <stdio.h>
#include "token.h"
#include "winp.h"
#include "fmacs.h"

/*
 * FCLEAR will clear the field from the current cursor position to
 * the end of the field
 */
fclear()
{
	register int row, col;
	register int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	for (row = saverow, col = savecol; row <= Lastrow; row++, col = 0) {
		fgo(row, col);
		for (; col <= Lastcol; col++)
			fputchar(' ');
	}
	fgo(saverow, savecol);
}

fclearline()
{
	register int col, savecol;

	savecol = Cfld->curcol;
	for (col = savecol; col <= Lastcol; col++)
		fputchar(' ');
	fgo(Cfld->currow, savecol);
}
