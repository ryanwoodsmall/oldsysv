/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/softtext.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
extern struct window wn;
extern struct device dv;

softtext(cdptr,table)
struct command *cdptr;
char *table[];
{	char *ptr,ch;
	int wt,st,col,tro,tsz,x0,y0;
	double xhch,yhch,xvch,yvch;
/*
 *  softtext draws and rotates text string pointed to by ptr
 *  from characters defined in table
 */
	tro=cdptr->trot; tsz=cdptr->tsiz; ptr=cdptr->tptr;
	wt = (cdptr->font >> 4) & 017;  st = cdptr->font & 017;
	col = cdptr->color;
	x0= *cdptr->aptr;  y0= *(cdptr->aptr+1);
	gslhchar(&xhch,&yhch,tro,.8*tsz);
	gslvchar(&xvch,&yvch,tro,1.0 * tsz);

	x0 -= (xhch - xvch)/2.; y0 -=(yhch - yvch)/2.;
	for(;*ptr != '\n' && *ptr != EOS;ptr++,x0+=(int)xhch,y0+=(int)yhch){
		if(*ptr=='\b') {x0 -= xhch *2; y0 -= yhch *2;}
		else if((ch=(*ptr & 0177)) >= '!')
				drawchar(x0,y0,tsz,tro,wt,st,col,table[ch - '!']);
			else if(!(ch=='\t' || ch== ' ')){x0 -= xhch; y0 -= yhch;}
	}
}

drawchar(x0,y0,tsz,tro,wt,st,col,ptr)
int x0,y0,tsz,tro,wt,st,col;
char *ptr;
{	int xprev,yprev,dc,move,tab;
	double dx,dy;
/*
 *  drawchar draws character from table pointed to by ptr
 */
	for(xprev=x0,yprev=y0,move=TRUE,dc=FALSE;
	  (tab = *ptr++ & 0377);xprev=(int)dx,yprev=(int)dy) {
		switch(tab){
		case 0360 :  move=TRUE; break;
		case 0340 :  dc=TRUE; break;
		default:   /*  4 bits x and 4 bits y  */
			dx = (((tab>>4) -1) & 017) * tsz/14.;
			dy = (tab & 017) * (tsz/14.);
			dx += x0;
			if(dc){dy=y0-dy; dc=FALSE;}  else dy+=y0;
			if(tro) gslswing((double)x0,(double)y0,&dx,&dy,tro);
			if(move) move=FALSE;
			else clipseg(xprev,yprev,(int)dx,(int)dy,wt,st,col);
		}
	}
	return;
}
