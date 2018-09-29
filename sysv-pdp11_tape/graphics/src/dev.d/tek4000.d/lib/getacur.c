static char SCCSID[]="@(#)getacur.c	1.1";
#include <stdio.h>
#include "tek.h"
#include "../include/termodes.h"

extern struct tekterm tm;
#define CHBIT2(i,x)  if((i >> 1) & 01) x = 512

getacur(x1,y1)
int *x1,*y1;
{	struct tekaddr tk;
/*
 *  getacur sets x1 and y1 to screen values of 
 *  the current alpha cursor position
 */
	RAWMODE;
	INQUIRE;
	getpos(&tk);
	COOKMODE;
	tekxy(x1,y1,&tk);
	CHBIT2(tk.ch,*x1);
	return;
}

