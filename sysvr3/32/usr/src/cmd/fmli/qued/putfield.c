/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.qued:src/qued/putfield.c	1.6"

#include <stdio.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "moremacros.h"
#include "terror.h"

putfield(fld, str)
ifield *fld;
char *str;
{
	ifield *savefield;
	char *fputstring();

	if (str == NULL)
		return;
	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	else if (!Cfld)			/* no current field */
		return;
	if (Flags & I_INVISIBLE) {
		Cfld = savefield;
		return;
	}
	Flags |= I_CHANGED;
	fgo(0, 0);			/* home the cursor */

	/*
	 * Free remains of a previous field value
	 */
	if (Value)
		free(Value);
	if (Scrollbuf)
		free_scroll_buf(Cfld);	/* if used, reset scroll buffers */

	/*
	 * If Value is LESS than the visible field size
	 * then allocate at least the field size
	 * otherwise strsave the passed value.
	 */
	if (strlen(str) < Fbufsize) {
		if ((Value = malloc(Fbufsize)) == NULL)
			fatal(NOMEM, nil);
		strcpy(Value, str);
	}
	else
		Value = strsave(str);

	Valptr = fputstring(Value);	/* update pointer into value */
	fclear();			/* clear the rest of field */
	fgo(0, 0);			/* home the cursor */
	if ((Flags & I_SCROLL) && Currtype == SINGLE) {
		/*
		 * HORIZONTAL SCROLLING
		 * initialize scroll buffer and copy string to it
		 */
		unsigned vallength, maxlength;

		vallength = strlen(Value);
		maxlength = max(vallength + 1, FIELDBYTES);
		growbuf(maxlength);
		strcpy(Scrollbuf, Value);
		free(Value);
		Valptr = Value = NULL;
	}
	setarrows();
	Cfld = savefield;
}
