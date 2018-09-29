/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tekalpha.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "termodes.h"

extern struct control ct;

tekalpha(cdptr)
struct command *cdptr;
{	int x1,y1;
/*
 *  tekalpha escapes to alpha mode and prints text
 *  at the location x1,y1 if cmd is within window
 */
	x1 = *cdptr->aptr; y1 = *(cdptr->aptr+1);
	if( tekmove(x1,y1) ) {
		tekflush();
		ALPHAMODE;
		printf("%s",cdptr->tptr);
	}
}
