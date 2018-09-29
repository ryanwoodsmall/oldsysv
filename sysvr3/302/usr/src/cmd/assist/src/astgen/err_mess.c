/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:err_mess.c	1.20"

#include "../forms/muse.h"
#include "tools.h"

#define	ST_PAGE	64	/*where [page 1 of 2] appears on select*/

char *errors[] =  {
/* 00 */  "",
/* 01 */  " '$ASSISTBIN' is not defined, and needs to be to run 'ASSIST' ",
/* 02 */  " 'assist' not found or not executable.  See your administrator ",
/* 03 */  " No line begins with that letter ",
/* 04 */  " Cannot open help file.  See your administrator ",
/* 05 */  " Command is not valid on this line ",
/* 06 */  " Unknown command ",
/* 07 */  " No help found for this item.  See your administrator ",
/* 08 */  " Hit any key to continue ",
/* 09 */  " Cannot open temporary file for writing.  See your administrator ",
/* 10 */  " Cannot open temporary file for reading.  See your administrator ",
/* 11 */  " Validation takes no arguments and has already been selected ",
/* 12 */  " Requires characters replacing ^P ",
/* 13 */  " Warning:  opposite in effect to another selected validation ",
/* 14 */  " Validation has not been selected yet ",
/* 15 */  " Must be an integer from 0 to 80 ",
/* 16 */  "",
/* 17 */  " Help messages are limited to 17 lines ",
/* 18 */  " No more than 75 characters allowed per line ",
/* 19 */  "",
/* 20 */  " ^X may not be used here ",
/* 21 */  " ^Z may not be used here ",
/* 22 */  " ^R may not be used here ",
/* 23 */  " ^E may not be used here ",
/* 24 */  " ^J (or <RETURN>) is illegal here ",
/* 25 */  " ^M may not be used here ",
/* 26 */  " ^Y must be used in pairs only ",
/* 27 */  " Requires list of characters ",
/* 28 */  " Requires list of quotes ",
/* 29 */  " Requires validation script ",
/* 30 */  " Requires list of strings ",
/* 31 */  " Requires list of formats ",
/* 32 */  " Meaningless without prefixes or suffixes. Validation deleted ",
/* 33 */  " Requires list separator ",
/* 34 */  " Must contain the letter A ",
/* 35 */  " Contains multiple non-escaped (by \\) A's ",
/* 36 */  " Shell expansion not used when user values don't appear on command line",
/* 37 */  " WARNING:  no conditional exit field has been specified ",
/* 38 */  " WARNING:  no exit message has been specified yet ",
/* 39 */  " WARNING:  no conditional exit values have been specified yet ",
/* 40 */  " Too many fields ",
/* 41 */  " Too many default input strings ",
/* 42 */  " Too many validation functions ",
/* 43 */  " Too many inter-field relationships specified ",
/* 44 */  " Either too many validation functions or too many exceptions in mapping module ",
/* 45 */  " You cannot change the location of bundled-option fields  ",
/* 46 */  " You cannot select the current field (marked with >>>) ",
/* 47 */  " Help messages can contain only printing characters, SPACE, RETURN, ^B or ^Y ",
/* 48 */  " WARNING: No lines longer than 5 characters. Message is considered empty",
/* 49 */  " ^B must be used in pairs only ",
/* 50 */  " WARNING:  Screen name truncated to 17 characters ",
/* 51 */  " The command name 'cd' may not be used here ",
/* 52 */  " The command name 'umask' may not be used here ",
/* 53 */  " Line length exceeds 512.  If shell script, store in external file "
};

err_rpt(code,bell) 
int code; /*error number*/
int bell; /*0 if no bell, 1 if bell should be sounded*/
{
	int y,x;
	int messg_loc = LINES-3; /*row number for message*/

	getyx(stdscr,y,x);
	switch(code)
	{
	case 0:
		draw_line(messg_loc);
		break;
	default:
		REV;
		mvaddstr(messg_loc,40-(strlen(errors[code]) / 2),errors[code]);
		NREV;
		break;
	} /*switch*/
	move(y,x);
	if (bell != 0)
		beep();
}

storeline(flag,row)
int flag, row;
{
	static chtype stline[80];
	chtype *c_pt;
	int j;

	if (row<0 || row>LINES-1) return(0);


	if (flag==0)	/* Store line */
	{
		c_pt = stline;
		for (j=0; j<80 && j<COLS; j++)
			*c_pt++ = mvinch(row,j);
		*c_pt = null;
	}

	else		/* Store line */
	{
		move(row,0);
		clrtoeol();
		mvaddstr(row,0,stline);
	}
}
