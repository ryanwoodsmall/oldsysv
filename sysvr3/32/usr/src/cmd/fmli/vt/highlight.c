/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/highlight.c	1.2"

#include	<curses.h>
#include	<term.h>
#include	"attrs.h"

chtype	Attr_list[NUM_ATTRS];

setvt_attrs()
{
	static chtype	modes;

	if (enter_blink_mode)
		modes |= A_BLINK;
	if (enter_bold_mode)
		modes |= A_BOLD;
	if (enter_dim_mode)
		modes |= A_DIM;
	if (enter_reverse_mode)
		modes |= A_REVERSE;
	if (enter_standout_mode)
		modes |= A_STANDOUT;
	if (enter_underline_mode)
		modes |= A_UNDERLINE;
	Attr_list[ATTR_NORMAL] = A_NORMAL;
	Attr_list[ATTR_HIGHLIGHT] = modes & A_STANDOUT;
	if (modes & A_REVERSE)
		Attr_list[ATTR_HIGHLIGHT] = A_REVERSE;
	Attr_list[ATTR_VISIBLE] = Attr_list[ATTR_SHOW] = Attr_list[ATTR_UNDERLINE] = Attr_list[ATTR_SELECT] = Attr_list[ATTR_HIDE] = Attr_list[ATTR_HIGHLIGHT];
	if (modes & A_DIM)
		Attr_list[ATTR_SELECT] = Attr_list[ATTR_HIDE] = modes & (A_REVERSE | A_DIM);
	else if (modes & A_BOLD) {
		Attr_list[ATTR_HIGHLIGHT] |= A_BOLD;
		Attr_list[ATTR_SELECT] = A_BOLD;
	}
	if (modes & A_BLINK)
		Attr_list[ATTR_VISIBLE] |= A_BLINK;
	Attr_list[ATTR_MARK] = Attr_list[ATTR_SELECT];
	if (modes & A_UNDERLINE) {
		Attr_list[ATTR_MARK] = A_UNDERLINE;
		Attr_list[ATTR_UNDERLINE] = A_UNDERLINE;
	}
}
