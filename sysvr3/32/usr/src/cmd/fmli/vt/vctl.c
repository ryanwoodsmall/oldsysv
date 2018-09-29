/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/vctl.c	1.5"

#include	<curses.h>
#include	<stdio.h>
#include	<varargs.h>
#include	"wish.h"
#include	"ctl.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"attrs.h"
#include	"color_pair.h"

vt_ctl(vid, cmd, va_alist)
vt_id	vid;
unsigned	cmd;
va_dcl
{
	register struct vt	*v;
	register int	retval;
	va_list	args;
	int colpair, attr;

#ifdef _DEBUG

	if (vid < 0) {
		if ((vid = VT_curid) < 0)
			_debug(stderr, "NO CURRENT VT!\n");
	}
#else

	if (vid < 0)
		vid = VT_curid;
#endif

	v = &VT_array[vid];
	retval = SUCCESS;
	va_start(args);
	switch (cmd) {
	case CTSETATTR:
		attr = va_arg(args, int);
		colpair = va_arg(args, int);
		wattrset(v->win, highlights(attr) | COL_ATTR(colpair));
		break;
	case CTSETLIM:
		{
			extern int	VT_firstline;
			extern int	VT_lastline;

			VT_firstline = va_arg(args, int);
			VT_lastline = va_arg(args, int);
		}
		break;
	case CTGETCUR:
		retval = VT_curid;
		break;
	case CTGETITLE:
		*(va_arg(args, char **)) = v->title;
		break;
	case CTGETWDW:
		retval = v->number;
		break;
	case CTSETWDW:
		v->number = va_arg(args, int);
		v->flags |= VT_TDIRTY;
		break;
	case CTSETITLE:
		v->title = va_arg(args, char*);
		v->flags |= VT_TDIRTY;
		break;
	case CTSETARROWS:
		v->flags &= ~(VT_UPARROW | VT_DNARROW);
		v->flags |= va_arg(args, int) & (VT_UPARROW | VT_DNARROW);
		v->flags |= VT_ADIRTY;
		break;
	case CTGETVT:
		retval = vid;
		break;
	case CTGETSIZ:
		{
			int	*rows;
			int	*cols;
			int	r;
			int	c;

			rows = va_arg(args, int *);
			cols = va_arg(args, int *);
			getmaxyx(v->win, r, c);
			if (!(v->flags & VT_NOBORDER)) {
				r -= 2;
				c -= 2;
			}
			*rows = r;
			*cols = c;
		}
		break;
	case CTGETSTRT:
		{
			int	r;
			int	c;
			extern int	VT_firstline;

			getbegyx(v->win, r, c);
			if (!(v->flags & VT_NOBORDER)) {
				r++;
				c++;
			}
			r -= VT_firstline;
			*(va_arg(args, int *)) = r;
			*(va_arg(args, int *)) = c;
		}
		break;
	case CTGETPOS:
		{
			int	r;
			int	c;

			getyx(v->win, r, c);
			if (!(v->flags & VT_NOBORDER)) {
				r--;
				c--;
			}
			*(va_arg(args, int *)) = r;
			*(va_arg(args, int *)) = c;
		}
		break;
	case CTHIDE:
		{
			register vt_id	i;
			register struct vt	*v;

			wnoutrefresh(stdscr);
			for (i = VT_front; i != VT_UNDEFINED; i = v->next) {
				v = &VT_array[i];
				v->flags |= VT_DIRTY;
			}
		}
		break;
	default:
#ifdef _DEBUG
		_debug(stderr, "vt_ctl(%d, %d, ...) unknown command\n", vid, cmd);
#endif
		retval = FAIL;
		break;
	}
	va_end(args);
	return retval;
}
