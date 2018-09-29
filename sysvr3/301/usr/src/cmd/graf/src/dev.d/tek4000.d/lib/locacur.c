/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/locacur.c	1.2"
#include <stdio.h>
#include "tek.h"
#include "termodes.h"

extern struct tekterm tm;
locacur(x1,y1)
int x1,y1;
{	struct tekaddr tk;
	int exbyte;
/*
 *  locacur prints tek code to move to address given by 
 *  screen coordinates x and y, and sets alpha mode
 */
	xytekl(x1,y1,&tk,&exbyte);
	RAWMODE;
	GRAPHMODE;
	printf("%c%c%c%c%c",tk.yh,exbyte,tk.yl,tk.xh,tk.xl);
	COOKMODE;
	ALPHAMODE;
	return;
}
