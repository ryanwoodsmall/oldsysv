static char SCCSID[]="@(#)tekmove.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "../include/termodes.h"
#include "../../../../include/util.h"

extern struct control ct;
extern struct tekterm tm;
extern char obuf[],*optr;

tekmove(x,y)
int x,y;
{
/*
 *  tekmove produces scope code in obuf to move
 *  to the screen coordinates x,y
 */
	if(x<ct.w.lx || x>ct.w.hx 
	|| y<ct.w.ly || y>ct.w.hy) return(FAIL);
	*optr++ = '\035';   /* GS  */
	unscaleoff(&x,&y);
	addrbuf(x,y);
	return(SUCCESS);
}

tekerase()
{
	ERASE;
	tm.alphax=0; tm.alphay=755;
}
