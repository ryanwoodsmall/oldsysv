/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.form:src/form/fctl.c	1.4"

#include	<stdio.h>
#include	<varargs.h>
#include	"wish.h"
#include	"ctl.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"
#include	"vtdefs.h"

form_ctl(fid, cmd, va_alist)
form_id	fid;
unsigned	cmd;
va_dcl
{
	register struct form *f;
	register int	retval;
	va_list	args;

#ifdef _DEBUG
	if (fid < 0) {
		if ((fid = FORM_curid) < 0)
			_debug(stderr, "NO CURRENT FORM!\n");
	}
#endif
	f = &FORM_array[fid];
	retval = SUCCESS;
	va_start(args);
	switch (cmd) {
	case CTSETDIRTY:
		if (fid == FORM_curid)
			form_refresh(fid);	/* refresh now */
		else
			f->flags |= FORM_DIRTY;	/* refresh when made current */
		break;
	case CTSETPOS:
		{
			formfield ffld;

			ffld = (*(f->display))(va_arg(args, int), f->argptr);
			checkffield(f, &ffld);
			gotofield(*(ffld.ptr), va_arg(args, int), va_arg(args, int));
		}
		break;
	case CTSETPAGE:
		{
			wgo(0, 0);	 /* KLUGE (replace with ETI) */
			wclrwin();	 /* ... */ 
			f->flags |= FORM_ALLDIRTY;
			form_refresh(fid);
			break;
		}
	case CTGETARG:
		{
			char **strptr;

			strptr = va_arg(args, char **);
			if (*strptr == NULL)
				*strptr = (char *) getfield(NULL, NULL);
			else
				(void) getfield(NULL, *strptr);
		}
		break;
	case CTGETWDW:
		retval = vt_ctl(f->vid, CTGETWDW);
		break;
	case CTGETITLE:
		retval = vt_ctl(f->vid, CTGETITLE, va_arg(args, char *));
		break;
	case CTGETVT:
		retval = f->vid;
		break;
	case CTGETPARMS:
		*(va_arg(args, int *)) = f->rows;
		*(va_arg(args, int *)) = f->cols;
		break;
	case CTSETPARMS:
		f->rows = va_arg(args, int);
		f->cols = va_arg(args, int);
		f->flags |= FORM_DIRTY;
		break;
	case CTSETSHAPE:
		{
			int	srow, scol, rows, cols;

			srow = va_arg(args, int);
			scol = va_arg(args, int);
			rows = va_arg(args, int);
			cols = va_arg(args, int);
			if (srow >= 0)
				_form_reshape(fid, srow, scol, rows, cols);
		}
		break;
	default:
#ifdef _DEBUG
		_debug(stderr, "form_ctl(%d, %d, ...) unknown command\n", fid, cmd);
#endif
		retval = FAIL;
		break;
	}
	va_end(args);
	return retval;
}
