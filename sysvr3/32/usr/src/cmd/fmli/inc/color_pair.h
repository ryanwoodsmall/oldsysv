/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.inc:src/inc/color_pair.h	1.3"

/*
 * NOTE:
 *
 * According to the curses documentation an application should not 
 * depend on more than 7 color pairs for portability .... FMLI defines 
 * 8 pairs ....
 *
 * To account for this, the SLK_PAIR will be color pair 7 ... 
 * If the terminal does not support more than 7 color pairs
 * than pairs 8 and greater will be "monochrome" ....
 *
 * The hp color terminal is one such terminal that only supports 7
 * color pairs, therefore the SLKS will have no color.
 *
 * The macro CHK_PAIR() accounts for this distinction.
 */ 

/* definable color pairs */
#define WINDOW_PAIR		1
#define ACTIVE_TITLE_PAIR	2
#define INACTIVE_TITLE_PAIR	3
#define ACTIVE_BORD_PAIR	4
#define INACTIVE_BORD_PAIR	5
#define BANNER_PAIR		6
#define BAR_PAIR		7
#define SLK_PAIR		8

extern int Color_terminal;

#define CHK_PAIR(x) \
	(x > COLOR_PAIRS ? 0 : COLOR_PAIR(x))

#define COL_ATTR(x) \
	(Color_terminal == TRUE && COLOR_PAIRS >= 7 ? CHK_PAIR(x) : 0) 
