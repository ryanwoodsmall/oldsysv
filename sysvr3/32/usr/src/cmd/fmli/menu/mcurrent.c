/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.menu:src/menu/mcurrent.c	1.5"

#include	<stdio.h>
#include	"wish.h"
#include	"menu.h"
#include	"menudefs.h"
#include	"vtdefs.h"
#include	"attrs.h"
#include	"ctl.h"
#include	"color_pair.h"

#define menu_pos(m, i, r, c, dr, dc)	((m->ncols > 1) ? (dc = i / r, dr = i % r) : (dc = 0, dr = i - m->topline))

menu_id	MNU_curid = -1;
void	menu_index();

int
menu_current(mid)
menu_id	mid;
{
	register struct menu	*m;

	m = &MNU_array[mid];
	vt_current(m->vid);
	menu_index(m, m->index, m->hcols);
	MNU_curid = mid;
	return SUCCESS;
}

int
menu_noncurrent()
{
	register struct menu	*m;

	if (MNU_curid < 0)
		return FAIL;
	m = &MNU_array[MNU_curid];
	menu_index(m, m->index, -1);
	m->hcols = MENU_ALL;
	_menu_cleanup();
	MNU_curid = -1;
	return SUCCESS;
}

void
menu_index(m, newindex, hcols)
register struct menu	*m;
int	newindex;
int	hcols;
{
	register int	col;
	register int	line;
	int	cwidth;
	int	huse;
	int	duse;
	int	destrow;
	int	destcol;
	int	rows;
	int	cols;
	int	scol;
	int	sind;
	struct menu_line	ml;
	void	menu_show();

	vt_ctl(m->vid, CTGETSIZ, &rows, &cols);
	huse = min(m->hwidth, cwidth = (cols - 1) / m->ncols);
	huse = min(huse, cols - 2);
	if (m->dwidth)
		duse = max(0, cwidth - huse - 3);
	else
		duse = 0;
	/* remove old indicator (if any) */
	if (m->hcols >= 0 && m->index != newindex && m->index >= 0) {
		menu_pos(m, m->index, rows, cols, destrow, destcol);
		ml = (*m->disp)(m->index, m->arg);
		wgo(destrow, destcol * cwidth);
		menu_show(m, &ml, huse, duse, 0);
	}
	m->index = newindex;
	menu_pos(m, m->index, rows, cols, destrow, destcol);
	if ((m->flags & MENU_DIRTY) || destrow < -1 || destrow > rows) {
		/* desired index is far away - redraw menu with it in center */
		wgo(0, 0);
		wclrwin();
		m->topline = min(m->number - rows, m->index - rows / 2);
		if (m->ncols > 1 || m->index < rows)
			m->topline = 0;
		scol = 0;
		sind = m->topline;
		for (col = 0; col < m->ncols; col++) {
			for (line = 0; line < rows; line++) {
				ml = (*m->disp)(line + sind, m->arg);
				if (ml.highlight == NULL)
					break;
				wgo(line, scol);
				menu_show(m, &ml, huse, duse, 0);
			}
			scol += cwidth;
			sind += rows;
		}
		m->flags &= ~MENU_DIRTY;
		menu_pos(m, m->index, rows, cols, destrow, destcol);
	}
	else if (destrow == rows) {
		wscrollwin(1);
		m->topline++;
		destrow--;
	}
	else if (destrow == -1) {
		wscrollwin(-1);
		m->topline--;
		destrow++;
	}
	if (m->index >= 0) {
		ml = (*m->disp)(m->index, m->arg);
		wgo(destrow, destcol * cwidth);
		menu_show(m, &ml, huse, duse, hcols);
		m->hcols = hcols;
	}
	/* update arrows */
	if (m->ncols == 1) {
		line = 0;
		if (m->topline)
			line |= VT_UPARROW;
		if (m->topline + rows < m->number)
			line |= VT_DNARROW;
		vt_ctl(m->vid, CTSETARROWS, line);
	}
}

static void
menu_show(m, ml, len1, len2, high)
register struct menu	*m;
register struct menu_line	*ml;
int	len1;
register int	len2;
int	high;
{
	register char	*s, ch;
	register int	tot;
	int	r;
	int	c;

	if (ml->flags & MENU_MRK)
		ch = '*';
	else
		ch = (high ? MENU_MARKER : ' ');   /* really MENU_SELECTOR */
	wputchar(ch, ATTR_NORMAL);
	s = ml->highlight;
	tot = min(len1, strlen(s));
	if (high > 0) {
		vt_ctl(m->vid, CTSETATTR, ATTR_SELECT, BAR_PAIR);
		r = min(high, tot);
		for ( ; r > 0; r--, tot--, len1--)
			wputchar(*s++, ATTR_NORMAL);
		vt_ctl(m->vid, CTGETPOS, &r, &c);
		vt_ctl(m->vid, CTSETATTR, ATTR_NORMAL, WINDOW_PAIR);
	}
	for ( ; tot-- > 0; len1--)
		wputchar(*s++, ATTR_NORMAL);
	vt_ctl(m->vid, CTSETATTR, ATTR_NORMAL, WINDOW_PAIR);
	while (len1-- > 0)
		wputchar(' ', ATTR_NORMAL);
	s = ml->description;
	if (s && len2 > 0) {
		wputs(" - ");
		for (tot = min(len2, strlen(s)); tot > 0; tot--, len2--)
			wputchar(*s++, ATTR_NORMAL);
		while (len2-- > 0)
			wputchar(' ', ATTR_NORMAL);
	}
	if (high)
		wgo(r, c);
}


/* 
 * returns the "translated" menu item name (lininfo) or the
 * actual menu item name if the translated name is not provided.
 */
char *
menu_list(m, i)
struct menu	*m;
int i;
{
	struct menu_line ml;

	ml = (*m->disp)(i, m->arg);
	if (!(ml.flags & MENU_MRK))
		return(NULL);
	if (ml.lininfo && *(ml.lininfo) != '\0')
		return(ml.lininfo);
	else
		return(ml.highlight);
}
