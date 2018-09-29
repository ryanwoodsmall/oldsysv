/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:move.c	1.7"

#include "../forms/muse.h"
#include "tools.h"

VOID hilight(y,x,str)	/*moves to y,x and displays str with > */
int y;
int x;
char *str;
{
	standout();
	mvaddstr(y,x,str);
	mvaddch(y,x,'>');	/*cursor character*/
	standend();
	move(y,x);
}

int firstlet(stname,maxnum,cmd,loc) /*letter and current variable number*/
					/*returns item number*/
struct labels *stname; /*label info*/
int maxnum; /*number of items in stname*/
int cmd; /*letter user typed*/
int loc; /*current position number*/
{
	int spot;
	int found = FALSE;

	if ((cmd >= 'a' && cmd <= 'z') || (cmd >= 'A' && cmd <= 'Z'))
		{
		if (loc <= maxnum)
			spot = loc+1; /*mark current location*/
		else spot = 0;
		while (found == FALSE && spot <= maxnum-1)
			if (tolower(stname[spot].name[1]) == tolower(cmd))
				found = TRUE; /*match*/
			else ++spot; /*look at next item*/
		if (spot >= maxnum) /*time to wrap to beginning*/
			spot = 0;
		while (found == FALSE && spot <= loc)
			if (tolower(stname[spot].name[1]) == tolower(cmd))
				found = TRUE; /*found a match*/
			else ++spot;
		if (found == FALSE) /*out of loop*/
			{
			err_rpt(3,TRUE); /*and didn't find anything*/
			return(-1);
			}
		else return(spot); /*return location of match*/
		}
	else 		/*not between a-z or A-Z*/
		{
		err_rpt(6,TRUE); /*complain*/
		return(-1);
		}
}


int prevvar(maxnum,fld)		/*return number of next lowest location*/
int maxnum; /*num labels possible*/
int fld; /*current field*/
{
	if (fld > 0)
		fld--;
	else fld = maxnum-1;
	return(fld);
}

int nextvar(maxnum,fld) 
int maxnum; /*num of labels*/
int fld; /*current field*/
{
	if (fld < maxnum-1)
		fld++;
	else fld = 0;
	return(fld);
} /*end nextvar*/
