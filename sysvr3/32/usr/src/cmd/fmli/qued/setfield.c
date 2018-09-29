/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/setfield.c	1.2"

#include <stdio.h>
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"

#define FSIZE(x)	(x->rows * (x->cols + 1))

setfieldflags(fld, flags)
register ifield *fld;
register int flags;
{
	fld->flags = (flags & I_CHANGEABLE) | (fld->flags & ~(I_CHANGEABLE));
	if (fld->flags & I_INVISIBLE) {
		if ((fld->value = (char *) malloc(FSIZE(fld))) == NULL)
			fatal(NOMEM, "");
		fld->valptr = fld->value;
	}
}

/* LES: functions never called
setfieldpos(fld, row, col)
ifield *fld;
int row;
int col;
{
	fld->currow = row;
	fld->curcol = col;
}

ifield *
getcurfield()
{
	return(Cfld);
}
*/
