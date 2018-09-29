/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/teklines.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#define PIby2 1.570796326794896619
#define MAXX 1023
#define MAXY 767
#define BW 4
#define BW2 2.5

extern struct tekterm tm;
extern char *optr;

teklines(cdptr)
struct command *cdptr;
{	int x0,y0,x1,y1,px1,py1,*lptr,cnt;
/*
 *  teklines produces scope code in obuf to draw consecutive vectors
 *  of line weight cdptr->weight and line style cdptr->style beginning from
 *  to the points contained in the array pointed to by cdptr->aptr
 */
 	if(cdptr->style > LONGDASH) cdptr->style = SOLID;
	if(cdptr->style != tm.curls || cdptr->weight !=  tm.curlw)  
		newls(cdptr->style,cdptr->weight);
	lptr=cdptr->aptr;
	x0  = *lptr++;  y0 = *lptr++;
	for(cnt=cdptr->cnt-4; cnt>0; cnt-=2,x0=px1,y0=py1) {
		x1 = px1 = *lptr++;  y1 = py1 = *lptr++;
		unscaleoff(&x0,&y0);
		unscaleoff(&x1,&y1);
		if(clip(&x0,&y0,&x1,&y1,BW,BW,MAXX-BW,MAXY-BW)) {
			line(x0,y0,x1,y1);
			if( cdptr->weight==BOLD ) bold(x0,y0,x1,y1);
		}
	}
	tekflush();
}
newls(ls,lw)
int ls,lw;
{	char ch;
/*  newls generates scope code to change line styles  
 */
	*optr++ = '\035';  /*  gs  */
	*optr++ = '\033';  /*  esc  */
	if( lw==MEDIUM||lw==BOLD ) ch = 'h';
	else if (lw == INVISIBLE) ch = 'p';
	else ch = '`';
	*optr++ = ls + ch;
	tm.curls = ls;  tm.curlw = lw;
}
static
line(x0,y0,x1,y1) /* output code for a line */
int x0, y0, x1, y1;
{
	*optr++ = '\035';
	addrbuf(x0,y0);
	addrbuf(x1,y1);
}
static
bold(x0,y0,x1,y1) /* generate parallels */
int x0, y0, x1, y1;
{
	double ang, atan2(), sin(), cos();
	int dx, dy, i;

	if ( (y1-y0) == 0 )
		ang = PIby2;
	else
		ang = atan2((double)(y1-y0),(double)(x1-x0)) + PIby2; /* width measured perpendicular to line */
	dx = cos(ang)*BW2; dy = sin(ang)*BW2;
	for( i=2; i>0; i-- ) {
		line(x0-dx,y0-dy,x1-dx,y1-dy);
		line(x0+dx,y0+dy,x1+dx,y1+dy);
		dx/=2; dy/=2;
	}
}
