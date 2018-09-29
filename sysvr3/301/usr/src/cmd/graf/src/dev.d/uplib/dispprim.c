/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/dispprim.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
extern struct window wn;
extern struct device dv;

dispprim(cdptr,chtable)
char *chtable;
struct command *cdptr;
{
/*
 *  dispprim displays the primitive whose parameters are in the
 *  structure pointed to by cdptr
 */
	if (dv.hbuff)		/*  If device has hardware buffering, call buffering  */
		devbuff();	/*  routine before processing command.  */

	switch(cdptr->cmd) {
	case LINES: displines(cdptr); break;
	case TEXT: 
		if(dv.htext){
			devtext(*cdptr->aptr,*(cdptr->aptr+1),
			  cdptr->tptr,cdptr->tsiz,
			  cdptr->trot,.8*cdptr->tsiz,
			  cdptr->font,cdptr->color);
		}
		else softtext(cdptr,chtable); 
		break;
	case ARCS: 
		if (cdptr->style & 8)  {
			cdptr->style &= 7;
			if (dv.hfill)  devfill();
			else arcfill(cdptr);
		}
		if(dv.harc && cdptr->cnt == 8){
			devarc(*cdptr->aptr,*(cdptr->aptr+1),
			  *(cdptr->aptr+2),*(cdptr->aptr+3),
			  *(cdptr->aptr+4),*(cdptr->aptr+5),
			  cdptr->weight,cdptr->style,cdptr->color);
		}
		else softarc(cdptr);
		break;
	case ALPHA: harddisp(cdptr); break;
	}
	return;
}
displines(cdptr)
struct command *cdptr;
{	int x0,y0,x1,y1,*p,cnt;

	p= cdptr->aptr;
	if((cnt=cdptr->cnt-4)<2) return(FAIL);
	if (cdptr->style & 8)  {
		cdptr->style &= 7;
		if (dv.hfill)  devfill();
		else linefill(cdptr);
	}
	for(x0= *p++,y0= *p++,x1= *p++,y1= *p++;
	  cnt>=2; x0= x1,y0= y1,x1= *p++,y1= *p++,cnt-= 2){
		clipseg(x0,y0,x1,y1,cdptr->weight,
		  cdptr->style,cdptr->color);
	}
	return(SUCCESS);
}

harddisp(cdptr)
struct command *cdptr;
{	int cnt;
	char *p;

	p= cdptr->tptr;
	if((cdptr->cnt-3)<=0) return(FAIL);
	devcursor(*cdptr->aptr,*(cdptr->aptr+1));
	for(cnt=cdptr->cnt-3; cnt>0; cnt--){
		putchar(*p++); putchar(*p++);
	}
	return(SUCCESS);
}
