static char SCCSID[]="@(#)tekalpha.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "../include/termodes.h"

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
