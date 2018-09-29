static char SCCSID[]="@(#)locacur.c	1.1";
#include <stdio.h>
#include "tek.h"
#include "../include/termodes.h"

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
