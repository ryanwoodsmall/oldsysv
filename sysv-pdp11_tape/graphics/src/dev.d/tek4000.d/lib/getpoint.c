static char SCCSID[]="@(#)getpoint.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "../../../../include/util.h"
#include "../include/termodes.h"

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

