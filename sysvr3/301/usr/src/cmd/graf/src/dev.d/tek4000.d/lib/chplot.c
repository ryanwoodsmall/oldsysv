/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/chplot.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "util.h"
#include "debug.h"
#include "gsl.h"
#include "gpl.h"
#include "gedstructs.h"
#define RADIAN			57.2957795131
#define XSPRAT		(0.8)
#define XWRAT		(0.57)
double dx,dy;

chplot(cdptr,table)
struct command *cdptr;
char *table[];
{	char *ptr,ch;
	double sin(),cos(),xsp,ysp;
	long xlng, ylng;
/*
 *  chplot draws and rotates text string pointed to by ptr
 *  from characters defined in table
 */
	if(cdptr->tsiz<0) chfake(cdptr);
	else{
		xysp(&xsp,&ysp,cdptr->tsiz,cdptr->trot);
		cdptr->weight = (cdptr->font >> 4) & 017;
		cdptr->style = cdptr->font & 017;
		for(ptr=cdptr->tptr;*ptr != '\n' && *ptr != EOS;ptr++){
			if(*ptr=='\b'){   /*  BS  */
				*cdptr->aptr -= xsp;
				*(cdptr->aptr+1) -= ysp;
				continue;
			}
			if((ch=(*ptr & 0177)) >= '!')
				drawchar(cdptr,table[ch - '!']);
			if((xlng = *cdptr->aptr+xsp) > XYMAX ||
					(ylng = *(cdptr->aptr+1)+ysp) > XYMAX)
				break;
			else {
				*cdptr->aptr = xlng;
				*(cdptr->aptr+1) = ylng;
			}
		}
	}
}

drawchar(cdptr,ptr)
struct command *cdptr;
char *ptr;
{
	double cos(), sin();
	int tab,dc,leap;
	int x0,y0,x1,y1,xorig,yorig;
	long xlng, ylng;
/*
 *  drawchar draws character from table pointed to
 *  by ptr
 */
	xorig = x1 = *cdptr->aptr;  yorig = y1 = *(cdptr->aptr+1);
	cdptr->aptr = cdptr->array;
	x1 -= .5 * XWRAT * cdptr->tsiz;
	y1 -= .5 * cdptr->tsiz;
	if(cdptr->trot){
		dx=x1; dy=y1;
		gslswing((double)xorig,(double)yorig,&dx,&dy,cdptr->trot);
		x1=dx; y1=dy;
	}
	for(leap=TRUE,dc=FALSE;(tab = *ptr++ & 0377);) {
		if(tab == 0360) {leap = TRUE; continue;}  /*  leap  */
		if(tab == 0340) {dc = TRUE; continue;}  /*  decender  */
					/* left 4 bits are x and
					   right 4 are y         */
		x0 = (((tab>>4) -1) & 017) * cdptr->tsiz/14.;
		y0 = (tab & 017) * (cdptr->tsiz/14.);
		x0 = ((xlng=(long)x0 + x1) > XYMAX)?XYMAX:xlng;
		if(dc){y0 =y1-y0; dc=FALSE;}
		else y0 = ((ylng=(long)y0 + y1) > XYMAX)?XYMAX:ylng;
		if(cdptr->trot) {
			dx=x0; dy=y0;
			gslswing((double)x1,(double)y1,&dx,&dy,cdptr->trot);
			x0=dx; y0=dy;
		}
		if(!leap ){
			*cdptr->aptr++ = x0;  *cdptr->aptr++ = y0;
		}
		else{
			cdptr->cnt= cdptr->aptr-cdptr->array+1;
			cdptr->aptr=cdptr->array;
			teklines(cdptr);  
			*cdptr->aptr++ = x0;  *cdptr->aptr++ = y0;
			leap = FALSE;
		}
	}
	cdptr->cnt= cdptr->aptr-cdptr->array + 1;
	cdptr->aptr=cdptr->array;
	teklines(cdptr);  
	*cdptr->aptr = xorig;  *(cdptr->aptr+1) = yorig;
	return;
}

chfake(cdptr)
struct command *cdptr;
{	double sin(), cos();
	int x1,y1,sz;
/*
 *  chfake draws fish symbol surrounding text area when text is killed
 */
	sz= ABS(cdptr->tsiz);
	x1= *cdptr->aptr; y1= *(cdptr->aptr+1);
	cdptr->aptr = cdptr->array;
	cdptr->weight=NARROW; cdptr->style=SOLID;
	x1 -= .5 * XWRAT * sz;
	y1 -= .5 * sz;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;       /*  first location  */
	x1 += sz * XSPRAT;  y1 += sz;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;       /*  diag to up rt  */
	x1 += (gslslen(cdptr->tptr)-1) * sz * XSPRAT;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;        /*  top hor line  */
	y1 -= sz;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;     /*  vertical right end  */
	x1 -= (gslslen(cdptr->tptr)-1) * sz * XSPRAT;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;     /*  hor bottom line  */
	x1 -= sz * XSPRAT;  y1 += sz;
	*cdptr->aptr++ = x1;  *cdptr->aptr++ = y1;     /*  diagonal up & left at 1st char  */
	cdptr->cnt=cdptr->aptr-cdptr->array+2;
	cdptr->aptr=cdptr->array;
	if(cdptr->trot)swingfake(cdptr);
	teklines(cdptr);  
}

swingfake(cdptr)
struct command *cdptr;
{	int *ptr,cnt,sz;
	double x0,y0;
/*
 *  swingfake swings fish symbol points in cdptr by angle in cdptr->trot
 *  around center of first char
 */
	sz= ABS(cdptr->tsiz);
	ptr=cdptr->aptr;
	x0= *ptr;  y0= *(ptr+1);
	x0 += .5 * XWRAT * sz;
	y0 += .5 * sz;
	for(cnt=cdptr->cnt-2;cnt>0;cnt-=2,ptr+=2){
		dx= *ptr;  dy= *(ptr+1);
		gslswing(x0,y0,&dx,&dy,cdptr->trot);
		*ptr=dx; *(ptr+1)=dy;
	}
	return;
}
