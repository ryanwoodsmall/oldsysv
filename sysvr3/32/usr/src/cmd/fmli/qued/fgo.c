/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/fgo.c	1.1"

#include <stdio.h>
#include "token.h"
#include "winp.h"

fgo(row, col)
int row;
int col;
{
	Cfld->currow = row;
	Cfld->curcol = col;
	wgo(row + Cfld->frow, col + Cfld->fcol);
}
