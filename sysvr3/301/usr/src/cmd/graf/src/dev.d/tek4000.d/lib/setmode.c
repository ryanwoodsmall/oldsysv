/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/setmode.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "termodes.h"
extern struct tekterm tm;

setmode(mode)
int mode;
{
/*
 *  setmode saves current alpha cursor, sets rawmode,
 *  and sets terminal to gin or graph mode
 */

	if(tm.curmode != ALPHA) restoremode();
	getacur(&tm.alphax,&tm.alphay);
	switch(mode){
	case GIN:
		tm.curmode = GIN;
		RAWMODE;
		GINMODE;
		break;
	case GRAPH:
		tm.curmode = GRAPH;
/*		RAWMODE;*/
		break;
	case ALPHA:
		restoremode(); /* uses current cursor pos */
		break;
	}
	return;
}

restoremode()
{
/*
 *  restoremode  sets terminal to alpha mode if it isnt
 *  already in alpha mode.
 */

/*	sleep(1); /* wait for output to complete */
	tm.curmode = ALPHA;
	locacur(tm.alphax,tm.alphay);
	SMALLFONT;
	return;
}
