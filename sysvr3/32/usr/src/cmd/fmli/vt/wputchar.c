/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/wputchar.c	1.3"

#include	<curses.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"vt.h"

void
wputchar(ch, attr)
char	ch;
unsigned attr;
{
	register struct vt	*v;
	register chtype	c;

	c = ch;
	v = &VT_array[VT_curid];
	v->flags |= VT_DIRTY;

	if ( ch > 037 && ch < 0177 )
	{
		waddch(v->win, c | highlights(attr));
		return;
	}

	switch (c) {
	case MENU_MARKER:
		c = ACS_RARROW;
/* les */
		waddch(v->win, c | highlights(attr));
/***/
		break;
	case '\n':
	case '\b':
	case '\t':
	case '\r':
		break;
	default:
		if (c < ' ')
			return;
		break;
	}
	if (!(v->flags & VT_NOBORDER)) {
		int	row, col;
		int	mr, mc;

		getyx(v->win, row, col);
/* les: never used
		getmaxyx(v->win, mr, mc);
*/
		switch (c) {
		case '\n':
			wmove(v->win, row + 1, 1);
			return;
		case '\b':
			wmove(v->win, row, col - 1);
			return;
		case '\t':
			wmove(v->win, row, (col + 8) & ~7);
			return;
		case '\r':
			wmove(v->win, row, 1);
			return;
		}
	}
/* les: move to top
	waddch(v->win, c | highlights(attr));
*/
}
