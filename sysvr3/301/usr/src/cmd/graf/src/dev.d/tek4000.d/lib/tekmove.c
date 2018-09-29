/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tekmove.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "termodes.h"
#include "util.h"

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
