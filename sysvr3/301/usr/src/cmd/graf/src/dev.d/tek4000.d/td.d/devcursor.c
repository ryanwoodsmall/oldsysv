/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/td.d/devcursor.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "td.h"
#include "dev.h"
#define ALPHAMODE   printf("\037")      /*  US  */
#define GRAPHMODE   printf("\035")      /*  GS  */
devcursor(x1,y1)
int x1,y1;
{	struct tekaddr tk;
	int exbyte;
/*
 *  devcursor prints tek code to move to address given by 
 *  screen coordinates x and y, and sets alpha mode
 */
	xytekl(x1,y1,&tk,&exbyte);
	GRAPHMODE;
	printf("%c%c%c%c%c",tk.yh,exbyte,tk.yl,tk.xh,tk.xl);
	ALPHAMODE;
	return;
}
