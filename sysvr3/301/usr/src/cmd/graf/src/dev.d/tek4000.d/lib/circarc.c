/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/circarc.c	1.4"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "util.h"
#define FAC   	(0.01745)
#define RADIAN	57.2957795131
struct command cd;
int cnt,ccw,circ,*ptr;
double x1,y1,x2,y2,x3,y3,xc,yc,rad,x1x2,y1y2,x1x3,y1y3,xpt,ypt;
double atan2(),sin(),cos(),sqrt();

circarc(cdptr)
struct command *cdptr;
{	int points,ang,ang1,ang2,ainc,lastptval=FALSE;
/*
 * circarc decomposes arcs of a circle into chord points using the
 * sin/cos function.
 */
	if(cdptr->cnt !=8) {teklines(cdptr);return;}
	ptr=cdptr->aptr;
	x1 = *ptr++;  y1 = *ptr++;
	x2 = *ptr++;  y2 = *ptr++;
	x3 = *ptr++;  y3 = *ptr++;
	x1x2 = x2-x1; y1y2 = y2-y1; x1x3 = x3-x1; y1y3 = y3-y1;
	if(!((x1x2 *y1y3)-(y1y2 * x1x3)) &&   /*  straight line  */
	  !(x1==x3 && y1==y3)){
		teklines(cdptr);
		return;
	}
	arcrad(); /*  sets xc,yc,cco,and rad */

	ptr=cd.aptr=cd.array; cnt=0;  /*  init output struct  */
 	if(cdptr->style > LONGDASH) cdptr->style = SOLID;
	cd.style=cdptr->style; cd.weight=cdptr->weight;
	ang1 = (atan2(y1-yc,x1-xc) * RADIAN) + 0.5;
	ang2 = (atan2(y3-yc,x3-xc) * RADIAN) + 0.5;
	if(ang1 < 0)ang1 += 360;  if(ang2 < 0)ang2 += 360;
	if(circ)ang2 = ang1 + 360;
	if(ccw)ang = ang2 - ang1;
	else	ang = ang1 - ang2;
	if(ang < 0)ang += 360;
	points=ang / (ainc = 6);  /*  chord pts every 6 degrees */
	if(!ccw)ainc *= -1;
	outpoint((int)x1,(int)y1);
	for(ang = ang1 + ainc; (--points > 0); ang += ainc) {
		xpt = xc + cos(ang * FAC) * rad;
		ypt = yc + sin(ang * FAC) * rad;
		if(!validpt(xpt,ypt)){
			if(lastptval) flushbuf();
			lastptval=FALSE;
		}else{
			outpoint((int)xpt,(int)ypt);
			lastptval=TRUE;
		}
	}
	outpoint((int)x3,(int)y3);  flushbuf();  /*  last point  */
	return;
}

arcrad()
{
	double den,fxc,fyc,v1,v2,dxx,dyy;
/*
 *  arcrad computes the center point
 *  radius and arc orientation
 */
	circ = 0;  ccw = 1;
	v1 = (x1x2 * x1x2) + (y1y2 * y1y2);
	v2 = (x1x3 * x1x3) + (y1y3 * y1y3);
	if(v2 == 0.0) {	  /*  process full circle  */
		circ = 1;
		xc = (x1 + x2)/2.0;
		yc = (y1 + y2)/2.0;
		dxx = x1 -xc;   dyy = y1 - yc;
		rad = sqrt((dxx * dxx) + (dyy * dyy));
	}
	else  {
		den = 2.0 * ((x1x2 * y1y3) - (y1y2 * x1x3));
		fxc = ((y1y3 * v1) - (y1y2 * v2))/den;
		fyc = ((x1x2 * v2) - (x1x3 * v1))/den;
		rad = sqrt((fxc * fxc) + (fyc * fyc));
		xc = fxc + x1;
		yc = fyc + y1;
	}
	if(circ == 0) {
		if(((x2 * y1y3)-(x3 *y1y2)-(x1 * (y3-y2))) <0) ccw=0;
	}
	return;
}

outpoint(x,y)
int x,y;
{
/*
 *  outpoint outputs chord segments as lines primitives     
 */
	*ptr++ = x; *ptr++ = y;
	if(cnt++ >MAXLINES){
		flushbuf();
		outpoint(x,y);
	}
	return;
}
flushbuf(){
		cd.cnt=ptr-cd.aptr+2;
		teklines(&cd); ptr=cd.aptr; cnt=0;
}
validpt(x,y)
double x,y;
{	if(x>XYMAX || x<XYMIN) return(FAIL);
	if(y>XYMAX || y<XYMIN) return(FAIL);
	return(SUCCESS);
}
