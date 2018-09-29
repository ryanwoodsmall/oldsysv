/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.menu:src/menu/stmenu.c	1.7"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"menu.h"
#include	"menudefs.h"
#include	"vtdefs.h"
#include	"token.h"
#include	"ctl.h"
#include	"moremacros.h"
#include	"message.h"

#define myisprint(C)	((C) >= ' ' && (C) <= '~')

static char	*curstring;

token
menu_stream(t)
register token	t;
{
	register int	newindex;
	register struct menu	*m;
	extern menu_id	MNU_curid;
	void	_menu_cleanup();
	char badmatch[PATHSIZ];

	if (MNU_curid < 0)
		return t;
	m = &MNU_array[MNU_curid];
	/* curstring is set if we partial-matched on the previous character */
	if (curstring) {
		if (myisprint((int) t) && (newindex = menu_match(m, curstring, m->hcols, t)) != FAIL) {
			menu_index(m, newindex, m->hcols + 1);
			return TOK_NEXT;
		}
		else if (t == TOK_BACKSPACE) {
			if (m->hcols > 1) {
				menu_index(m, m->index, m->hcols - 1);
				return TOK_NEXT;
			}
		}
		if (myisprint((int) t) && t != ' ') {
			sprintf(badmatch, "Can't match item starting with \"%.*s%c\"",
				 m->hcols, curstring, (char) t);
			mess_temp(badmatch);
			mess_lock();
			return(TOK_BADCHAR);
		}
	}

	/*
	 * NOTE: tokens that change the current menu item return
	 * TOK_NEXT so that application stream handler (objmenu_stream)
	 * is aware ...
 	 */		
	switch (t) {
	case ' ':
	case TOK_RIGHT:
		if (m->ncols > 1) {
			int	row, col;

			vt_ctl(m->vid, CTGETSIZ, &row, &col);
			if ((newindex = m->index + row) < m->number) {
				menu_index(m, newindex, MENU_ALL);
				t = TOK_NEXT;
			}
			break;
		}
		/* FALL THROUGH */
	case TOK_DOWN:
	case TOK_NEXT:
		if (m->index < m->number - 1) {
			menu_index(m, m->index + 1, MENU_ALL);
			t = TOK_NEXT;
		}
		else { 
			menu_index(m, 0, MENU_ALL);
			t = TOK_NEXT;
		}
		break;
	case TOK_BACKSPACE:
	case TOK_LEFT:
		if (m->ncols > 1) {
			int	row, col;
			
			vt_ctl(m->vid, CTGETSIZ, &row, &col);
			if ((newindex = m->index - row) >= 0) {
				menu_index(m, newindex, MENU_ALL);
				t = TOK_NEXT;
			}
			break;
		}
		/* FALL THROUGH */
	case TOK_UP:
	case TOK_PREVIOUS:
		if (m->index > 0) {
			menu_index(m, m->index - 1, MENU_ALL);
			t = TOK_NEXT;
		}
		else { 
			menu_index(m, m->number - 1, MENU_ALL);
			t = TOK_NEXT;
		}
		break;
	case TOK_RETURN:
	case TOK_OPEN:
	case TOK_ENTER:
		t = TOK_OPEN;
		break;
	case TOK_HOME:
		menu_index(m, m->topline, MENU_ALL);
		t = TOK_NEXT;
		break;
	case TOK_BEG:
		menu_index(m, 0, MENU_ALL);
		t = TOK_NEXT;
		break;
	case TOK_LL:
	case TOK_SHOME:
		{
			int	row, col;

			vt_ctl(m->vid, CTGETSIZ, &row, &col);
			menu_index(m, row - 1, MENU_ALL);
		}
		t = TOK_NEXT;
		break;
	case TOK_END:
		menu_index(m, m->number - 1, MENU_ALL);
		t = TOK_NEXT;
		break;
	case TOK_SR:
		if (m->topline > 0) {
			newindex = m->index - 1;
			/* force reverse scroll */
			menu_index(m, m->topline - 1, 0);
			/*reset position */
			menu_index(m, newindex, MENU_ALL);
			t = TOK_NEXT;
		}
		break;
	case TOK_SF:
		if (m->ncols == 1) {
			int	row;
			int	col;

			vt_ctl(m->vid, CTGETSIZ, &row, &col);
			if (m->topline + row < m->number) {
				newindex = m->index + 1;
				/* cause scroll */
				menu_index(m, m->topline + row, 0);
				/* reset position */
				menu_index(m, newindex, MENU_ALL);
				t = TOK_NEXT;
			}
		}
		break;
	case TOK_PPAGE:
	case TOK_NPAGE:
		if (m->ncols == 1) {
			int	row;
			int	col;

			vt_ctl(m->vid, CTGETSIZ, &row, &col);
			if (t == TOK_PPAGE)
				newindex = m->topline - row + row / 2;
			else
				newindex = m->topline + row + row / 2;
			if (newindex >= 0 && newindex < m->number) {
				/* force middle of page */
				menu_index(m, newindex, 0);
				/* set position to top of window */
				menu_index(m, m->topline, MENU_ALL);
				t = TOK_NEXT;
			}
		}
		break;
	default:
		if (myisprint((int) t)) {
			if ((newindex = menu_match(m, nil, 0, t)) != FAIL) {
				menu_index(m, newindex, 1);
				return TOK_NEXT;
			}
			else {
				sprintf(badmatch, "Can't match item starting with \"%c\"", (char) t);
				mess_temp(badmatch);
				mess_lock();
				return(TOK_BADCHAR);
			}
		}
		break;
	}
	_menu_cleanup();
	return t;
}

void
_menu_cleanup()
{
	if (curstring)
		free(curstring);
	curstring = NULL;
}

static
menu_match(m, s, n, t)
register struct menu	*m;
char	*s;
int	n;
token	t;
{
	register int	i;
	register int	count;
	register char	*p;
	void strtolower();

	for (i = m->index, count = 0; count < m->number; count++) {
		struct menu_line men;

		men = (*m->disp)(i, m->arg);
		p = men.highlight;
		if (nocase_strncmp(p, s, n) == 0 && ((p[n] == t) || 
				(isupper(t) && (p[n] == _tolower(t))) ||
				(islower(t) && (p[n] == _toupper(t))))) {
			if (m->index != i || curstring == NULL) {
				if (curstring)
					free(curstring);
				curstring = strsave(p);
				strtolower(curstring);
			}
			return i;
		}
		if (++i >= m->number)
			i = 0;
	}
	return FAIL;
}

static void 
strtolower(s)
register char *s;
{
	for (s; *s != '\0'; s++)
		*s = tolower(*s);
}

static int
nocase_strncmp(p, s, n)
register char *p, *s;
int n;
{
	register int i;

	if (!p || !s)
		return(1);
	for (i = 0; i < n; i++) {
		if (!(*p || *s))
			break;		/* both strings shorter than n */
		else if (tolower(*p++) != tolower(*s++))
			return(1);
	}
	return(0);
}
