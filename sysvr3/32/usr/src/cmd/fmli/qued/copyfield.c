/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/copyfield.c	1.4"

#include <stdio.h>
#include <memory.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include	"moremacros.h"

/*
 * COPYFIELD will copy a field form one part of the screen to another
 * (including all of the field status information)
 */
copyfield(srcfld, destfld) 
ifield *srcfld, *destfld;
{
	ifield *savefield;
	long tmpoffset;
	char *fputstring();

	if (srcfld == NULL || destfld == NULL)
		return(FAIL);
	savefield = Cfld;
	Cfld = destfld;
	if (srcfld->scrollbuf) {		/* if a scrollable field */
		register int linesize, i;

		if (destfld->scrollbuf)			/* ehr3 */
			free(destfld->scrollbuf);	/* ehr3 */

		destfld->scrollbuf = malloc(srcfld->buffsize);
		destfld->buffsize = srcfld->buffsize;
		memcpy(destfld->scrollbuf, srcfld->scrollbuf, srcfld->buffsize);
		linesize = destfld->cols + 1;
		tmpoffset = 0L;
		for (i = 0; i < srcfld->rows; i++) {
			/* print the copied field to the screen */
			fgo(i, 0);
			fputstring(destfld->scrollbuf + tmpoffset);
			tmpoffset += linesize;
		}
	}
	if (srcfld->value) {
		if (destfld->value)		/* ehr3 */
			free(destfld->value);	/* ehr3 */

		destfld->value = strsave(srcfld->value);

		if (!destfld->scrollbuf)	/* if not a scroll field */
			destfld->valptr = fputstring(destfld->value);
	}
	destfld->currow = srcfld->currow;
	destfld->curcol = srcfld->curcol;
	Cfld = savefield;
	return(SUCCESS);
}

/*
 * HIDEFIELD will remove the field from screen WITHOUT destroying the
 * ifield structure.
 */
hidefield(fld)
ifield *fld;
{
	ifield *savefield;

	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	setfieldflags(fld, fld->flags & ~I_FILL);
	fgo(0, 0);
	fclear();
	Cfld = savefield;
}
