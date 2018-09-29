/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.form:src/form/fcheck.c	1.2"

#include	<stdio.h>
#include	"wish.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"

checkffield(fptr, pffld)
struct form *fptr;
register formfield *pffld;
{
	register ifield *fld;
	ifield *newfld, *newfield();

	if (!(*(pffld->ptr)))   /* initialize field if this is the first time */
		*(pffld->ptr) = (char *) newfield(pffld->frow, pffld->fcol,
			pffld->rows, pffld->cols, pffld->flags);
	fld = (ifield *) *(pffld->ptr);
	if ((fptr->flags & FORM_ALLDIRTY) || (fld->value == NULL)
			|| (strcmp(fld->value, pffld->value) != 0))
		putfield(fld, pffld->value);
	if (((ifield *) *(pffld->ptr))->flags != pffld->flags)
		setfieldflags(*(pffld->ptr), pffld->flags);
}
