/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/getacur.c	1.2"
#include <stdio.h>
#include "tek.h"
#include "termodes.h"

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

