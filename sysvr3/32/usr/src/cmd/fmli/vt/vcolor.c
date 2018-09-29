/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/vcolor.c	1.6"

#include "curses.h"
#include "wish.h"
#include "color_pair.h"
#include "moremacros.h"

#define NUMDEFCOLORS	8
#define MAXCOLORS	64

static int Numcolors = NUMDEFCOLORS; 

/*
 * Table of known colors
 */
struct ctab {
	char *colorstr;
	int id;
} Color_tab[MAXCOLORS] = {
	{ "black", COLOR_BLACK },
	{ "blue", COLOR_BLUE },
	{ "green", COLOR_GREEN },
	{ "cyan", COLOR_CYAN },
	{ "red", COLOR_RED },
	{ "magenta", COLOR_MAGENTA },
	{ "yellow", COLOR_YELLOW },
	{ "white", COLOR_WHITE },
	{ NULL, 0 }
};

/*
 * SET_SCR_COLOR sets the screen background color and refreshes
 * the screen
 */
set_scr_color(colpair, dorefresh)
int colpair;
int dorefresh;
{
	wbkgd(stdscr, COL_ATTR(colpair));
	if (dorefresh)
		refresh();
}

/*
 * SET_SLK_COLOR simply sets the slk color pair
 */ 
set_slk_color(colpair)
{
	slk_attron(COL_ATTR(colpair));
}
	
/*
 * SETPAIR creates new color pair combinations
 */
setpair(pairnum, foreground, background)
int pairnum, foreground, background;
{
	if (foreground < 0)
		foreground = COLOR_WHITE;	
	if (background < 0)
		background = COLOR_BLACK;
	if (init_pair(pairnum, foreground, background) == ERR)
		init_pair(pairnum, COLOR_WHITE, COLOR_BLACK);
}

/*
 * SETCOLOR creates new color specifications or "tweeks" old ones.
 * (returns 1 on success and 0 on failure)
 */
setcolor(colorstr, r, g, b)
char *colorstr;
int r, g, b;
{
	register int cindex, id; 
	short oldr, oldg, oldb;
	int cant_init;

	if (!can_change_color())
		return(-1);
	cant_init = 0;
	if ((cindex = lookup_color(colorstr)) >= 0) {
		/*
		 * The color has been previously defined ...
		 * If you can't change the color specification then
		 * restore the old specification. 
		 */
		color_content(cindex, &oldr, &oldg, &oldb);
		if (init_color(cindex, r, g, b) == ERR) {
			cant_init++;
			if (init_color(cindex, oldr, oldg, oldb) == ERR)
				id = -1; 	/* just in case */
			else
				id = cindex;
		}
		else
			id = cindex;
		Color_tab[cindex].id = id;
	}
	else if ((cindex = add_color(colorstr)) >= 0) {
		/*
		 * The color is NEW ...
		 */
		if (init_color(cindex, r, g, b) == ERR)
			id = -1;
		else
			id = cindex;
		Color_tab[cindex].id = id;
	}
	else
		id = -1;
	return(cant_init ? 0 : (id >= 0));
} 

/*
 * GETCOLOR_ID returns the color identifier of the passed color string
 */
getcolor_id(colorstr)
char *colorstr;
{
	int index;

	index = lookup_color(colorstr);
	if (index >= 0)
		return(Color_tab[index].id);
	else
		return(-1);
}

/*
 * LOOKUP_COLOR returns the index of the passed color string from the
 * color table (or "-1" if the color is not in the table).
 */ 
static int
lookup_color(colorstr)
char *colorstr;
{
	register int i;

	/* put it in the color table */
	for (i = 0; i < Numcolors; i++) {
		if (strcmp(colorstr, Color_tab[i].colorstr) == 0)
			return(i);
	}
	return(-1);
}

/*
 * ADD_COLOR adds a new color to the color table if the number of colors
 * is less than COLORS (curses define for the number of colors the terminal
 * can support) and less than MAXCOLORS (color table size)
 */
static int 
add_color(colorstr)
char *colorstr;
{
	if (Numcolors < COLORS && Numcolors < MAXCOLORS) {
		Color_tab[Numcolors].colorstr = strsave(colorstr);
		return(Numcolors++);
	}
	else
		return(-1);
} 
