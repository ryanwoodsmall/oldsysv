/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/getpoint.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "util.h"
#include "termodes.h"

getpoint(x,y,ch) /* return char and universe location */
int *x,*y;
char *ch;
{
	struct tekaddr tk;
	setmode(GIN);
	getpos(&tk);
	restoremode();
	tekxy(x,y,&tk);
	scaleoff(x,y);
	if( (*ch=tk.ch)=='\177' || *ch=='\0' ) interr(); /* interrupt */
	return(SUCCESS);
}

