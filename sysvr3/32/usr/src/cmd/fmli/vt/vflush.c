/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/vflush.c	1.10"
static char sccsid[] = "@(#)vflush.c	1.3 = R1.0 1.12";

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"attrs.h"
#include	"color_pair.h"

extern int Refresh_slks;

extern int Color_terminal;

void
vt_flush()
{
	void	vt_display();
/*
	_debug3(stderr, "\t--==[ FLUSHING ]==--\n");
*/
	vt_display();
	if (Refresh_slks) {
		Refresh_slks = 0;
		slk_restore();	
	}
	if (VT_front >= 0)
		wnoutrefresh(VT_array[VT_front].win);
	doupdate();
/*
	_debug3(stderr, "\t--==[ FINISHED ]==--\n");
*/
}

static void
vt_display()
{
	register struct vt	*v;
	int colattr;
	vt_id	vid;
	void	vt_title();
	void	vt_arrow();

	for ( vid = VT_back; vid != VT_UNDEFINED; vid = v->prev )
	{
		v = &VT_array[vid];

		if ( !(v->flags & VT_ANYDIRTY))
			continue;
	
		if (!(v->flags & VT_NOBORDER)) {
			int	row, col;
	
			if (v->flags & VT_BDIRTY)
			{
				getyx(v->win, row, col);
				if (vid == VT_curid)
					colattr = ACTIVE_BORD_PAIR;
				else
					colattr = INACTIVE_BORD_PAIR;
				wattrset(v->win, A_NORMAL | COL_ATTR(colattr));
				box(v->win, 0, 0);
				wattrset(v->win, A_NORMAL | COL_ATTR(WINDOW_PAIR));
				v->flags |= VT_TDIRTY | VT_ADIRTY;
				wmove(v->win, row, col);
			}

			if (v->flags & VT_TDIRTY)
			{
				getyx(v->win, row, col);
				if (vid == VT_curid)
					vt_title(v, TRUE);
				else
					vt_title(v, FALSE);
				wmove(v->win, row, col);
			}

			if (v->flags & VT_ADIRTY)
			{
				getyx(v->win, row, col);
				if (vid == VT_curid)
					vt_arrow(v, TRUE);
				else
					vt_arrow(v, FALSE);
				wmove(v->win, row, col);
			}
		}
		{
			int	sr1, sc1, r1, c1;
			int	sr2, sc2, r2, c2;
			register vt_id	ov;
			register struct vt	*vp;
/*	
			_debug3(stderr, "flushing %d(#%d) flags = 0x%x\n", vid, v->number, v->flags);
*/
			wnoutrefresh(v->win);
			getbegyx(v->win, sr1, sc1);
			getmaxyx(v->win, r1, c1);
			for (ov = VT_front; ov != vid; ov = vp->next) {
				vp = &VT_array[ov];
				getbegyx(vp->win, sr2, sc2);
				getmaxyx(vp->win, r2, c2);
				if (_vt_overlap(sr1, r1, sr2, r2) && _vt_overlap(sc1, c1, sc2, c2))
					vp->flags |= VT_BDIRTY;
			}
		v->flags &= ~VT_ANYDIRTY;
		}
        }
}

static void
vt_arrow(v, active_flag)
register struct vt	*v;
int active_flag;
{
	int	r;
	int	c;
	int	colattr;

	getmaxyx(v->win, r, c);
	wmove(v->win, r - 1, c - 4);
	if (active_flag == TRUE)
		colattr = ACTIVE_BORD_PAIR;
	else
		colattr = INACTIVE_BORD_PAIR;
	wattrset(v->win, A_NORMAL | COL_ATTR(colattr));
	if (v->flags & VT_UPARROW)
		waddch(v->win, ACS_UARROW);
	else
		waddch(v->win, ACS_HLINE);
	if (v->flags & VT_DNARROW)
		waddch(v->win, ACS_DARROW);
	else
		waddch(v->win, ACS_HLINE);
	wattrset(v->win, A_NORMAL | COL_ATTR(WINDOW_PAIR));
}

static void
vt_title(v, active_flag)
register struct vt	*v;
int	active_flag;
{
	register char	*s;
	int	c;
	int	dummy;
	int	bl;
	int	const;

	if ((s = v->title) == NULL)
		s = nil;
	getmaxyx(v->win, dummy, c);
	/*
	 * const is # of columns taken up by corners and by number 
	 * displayed on title line
	 */
	if (v->number > 0)
		const = 5;
	else
		const = 2;
	bl = (c - const) / 2 - (strlen(v->title) + 1) / 2;
	if (bl < 0)
		bl = 0;
	c -= bl + const;
	if (active_flag)
		wattrset(v->win, highlights(ATTR_HIGHLIGHT) |
			COL_ATTR(ACTIVE_TITLE_PAIR));
	else
		wattrset(v->win, highlights(ATTR_HIDE) | 
			COL_ATTR(INACTIVE_TITLE_PAIR));
	wmove(v->win, 0, 1);
	if (v->number > 0)
		wprintw(v->win, "%2d %*s%-*.*s", v->number, bl, "", c, c, s);
	else
		wprintw(v->win, "%*s%-*.*s", bl, "", c, c, s);
	wattrset(v->win, A_NORMAL | COL_ATTR(WINDOW_PAIR));
}
