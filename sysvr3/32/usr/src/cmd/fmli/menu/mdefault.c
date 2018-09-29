/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.menu:src/menu/mdefault.c	1.6"

#include	<stdio.h>
#include	"wish.h"
#include	"menu.h"
#include	"menudefs.h"
#include	"vtdefs.h"
#include	"terror.h"
#include	"ctl.h"

menu_id	menu_make();

menu_id
menu_default(title, flags, startrow, startcol, disp, arg)
char	*title;
unsigned	flags;
int	startrow;
int	startcol;
struct menu_line	(*disp)();
char	*arg;
{
	return menu_make(-1, title, flags, startrow, startcol, disp, arg);
}

menu_id
menu_reinit(mid, flags, disp, arg)
menu_id	mid;
unsigned	flags;
struct menu_line	(*disp)();
char	*arg;
{
	char	*s;
	register menu_id	newmid;
	register vt_id	oldvid;
	register menu_id	oldmid;
	int	top, line;

	oldmid = MNU_curid;
	oldvid = vt_current(MNU_array[mid].vid);
	vt_ctl(VT_UNDEFINED, CTGETITLE, &s);
	newmid = menu_make(vt_ctl(VT_UNDEFINED, CTGETWDW), s, flags | VT_COVERCUR, VT_UNDEFINED, VT_UNDEFINED, disp, arg);
	menu_ctl(mid, CTGETPARMS, &top, &line);
	menu_close(mid);
	menu_ctl(newmid, CTSETPARMS, top, line);
	menu_current(newmid);
	if (MNU_array[mid].vid != oldvid) {
		menu_noncurrent();
		if (oldmid >= 0)
			menu_current(oldmid);
		else
			vt_current(oldvid);
	}
	return newmid;
}

static menu_id
menu_make(num, title, flags, startrow, startcol, disp, arg)
int	num;
char	*title;
unsigned	flags;
int	startrow;
int	startcol;
struct menu_line	(*disp)();
char	*arg;
{
	register int	total;
	register int	hwidth, dwidth;
	register int	mheight, mwidth;
	int	cols;
	vt_id	vid;
	struct menu_line	ml;

	/*
	 * start out assuming the following:
	 *	single column
	 *	length is number of items in menu
	 *
	 * after calculating these values, we adjust as necessary
	 */
	cols = 1;
	hwidth = dwidth = 0;
	ml = (*disp)(0, arg);

	for (total = 0; ml.highlight; ml = (*disp)(++total, arg)) {
		hwidth = max(hwidth, strlen(ml.highlight));
		if (ml.description)
			dwidth = max(dwidth, strlen(ml.description));
	}
	if (!total)
		return((menu_id) FAIL);

	mheight = total;
	/*
	 * if there's any description, stay single column,
	 *	width is longest highlight + longest description +
	 *		3 (for the " - ") + 2 (for space between text and sides)
	 * but don't let the menu get wider than 40 columns (by truncating
	 * from the right)
	 */
	if (dwidth)
		for (mwidth = hwidth + dwidth + 5; !fits(flags, 1, mwidth); mwidth--)
			;
	else {
		/*
		 * there's no description,
		 *	width is longest highlight + 2 (for space on sides)
		 *
		 * if there are more than 10 items, check to see if we want
		 * to go multi-column.  Criteria for this are:
		 *	make it at least as wide as the title, with a maximum
		 *		width of 64
		 * make its aspect as close to 1:3 (height:width) as possible).
		 *
		 * These are arbitrarily pleasing values.
		 */
		mwidth = hwidth + 2;
		if (total >= 10) {
			for (cols = 1; mwidth < 64; cols++) {
				mwidth = 1 + cols * (1 + hwidth);
				mheight = (total + cols - 1) / cols;
				if (mheight * 3 <= mwidth && mwidth >= strlen(title) + 3)
					break;
			}
			if (mwidth >= 64) {
				cols = max(cols - 1, 1);
				mwidth = 1 + cols * (1 + hwidth);
			}
			if (!fits(flags, mheight, mwidth)) {
				cols = 1;
				mwidth = min(hwidth + 2, 40);
				mheight = total;
			}
		}
	}
	mwidth = max(mwidth, strlen(title) + 3);
	if (cols == 1)
		mheight = min(mheight, 10);
	if ((vid = vt_create(title, flags, startrow, startcol, mheight, mwidth)) == VT_UNDEFINED) {
		
		/* 
		 * try putting the VT anywhere 
		 */
		vid = vt_create(title, flags, VT_UNDEFINED, VT_UNDEFINED, mheight, mwidth);
	}
	if (vid == VT_UNDEFINED) {
		mess_temp("Object can not be displayed, frame may be too large for the screen");
		return((menu_id) FAIL);
	}
	if (num >= 0)
		vt_ctl(vid, CTSETWDW, num);
	return(menu_custom(vid, flags, cols, hwidth, dwidth, total, disp, arg));
}
