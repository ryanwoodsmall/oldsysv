/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.form:src/form/frefresh.c	1.5"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "form.h"
#include "attrs.h"

form_refresh(fid)
form_id fid;
{
	register int i, maxrows, maxcols;
	register char *argptr;
	int curmaxrows, curmaxcols;
	formfield ff, (*disp)();
	struct form *fptr;
	vt_id oldvid;

	fptr = &FORM_array[fid];
	oldvid = vt_current(fptr->vid);
	disp = fptr->display;
	argptr = fptr->argptr;
	curmaxrows = fptr->rows;
	curmaxcols = fptr->cols;
	maxrows = maxcols = 0;
	for (i = 0, ff = (*disp)(0, argptr); ff.name != NULL; ff = (*disp)(++i, argptr)) {
		/*
		 * For all fields that are visible on the current page ...
		 */
		if (ff.nrow >= 0 && ff.ncol >= 0) {
			/*
			 * If there is an offset for the field name 
			 * then print it 
			 */
			wgo(ff.nrow, ff.ncol);
			wputs(ff.name);
		}
		if (ff.cols > 0 && ff.rows > 0 && ff.frow >= 0 && ff.fcol >= 0) {
			/*
			 * If there is an offset for the field itself AND
			 * the rows and columns are positive then post the
			 * field.
			 */ 
			checkffield(fptr, &ff);
			maxrows = max(maxrows, max(ff.frow + ff.rows, ff.nrow + 1));
			maxcols = max(maxcols, max(ff.fcol + ff.cols, ff.ncol + strlen(ff.name)));
		}
		if (maxrows > curmaxrows || maxcols > curmaxcols) {
			/*
			 * If the form should grow in size then reinitialize
			 * the form altogether.
			 */
			form_reinit(fid, fptr->flags, disp, argptr);
			fptr->flags &= ~(FORM_DIRTY | FORM_ALLDIRTY);
			return;
		}
	}
	fptr->flags &= ~(FORM_DIRTY | FORM_ALLDIRTY);
	gotofield(NULL, 0, 0);
	(void) vt_current(oldvid);
}
