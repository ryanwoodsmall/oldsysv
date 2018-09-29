/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/arcfill.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
#include "fill.h"
static int ccw,circ;
static double x1,y1,x2,y2,x3,y3,xc,yc,rad,x1x2,y1y2,x1x3,y1y3;
double ix1,ix2,iy1,iy2,slope,offset,incr;
extern struct device dv;

arcfill(cdptr)
struct command *cdptr;
{
	double ang1,ang2,ang3,ang4,tmp,angle,spacing,dis;
	int col,wt,st,*ptr;

	if (cdptr->cnt != 8)  {
		linefill(cdptr);
		return;
	}

	spacing = (dv.v.hx-dv.v.lx)/SPACING;

	ptr = cdptr->aptr;
	x1 = *ptr++; y1 = *ptr++;
	x2 = *ptr++; y2 = *ptr++;
	x3 = *ptr++; y3 = *ptr;

	x1x2 = x2-x1; y1y2 = y2-y1; x1x3 = x3-x1; y1y3 = y3-y1;
	if(!((x1x2 *y1y3)-(y1y2 * x1x3)) &&   /*  straight line  */
	  !(x1==x3 && y1==y3)){
		linefill(cdptr);
		return;
	}
	arcrad(); /*  sets xc,yc,cco,and rad */

	switch (cdptr->color)  {
		case BLACK:	angle = BCKANG; break;
		case RED:		angle = REDANG; break;
		case GREEN:	angle = GRNANG; break;
		case BLUE:	angle = BLUANG; break;
		default:		angle = BCKANG; 
	}

	dis = sqrt((y1-yc)*(y1-yc) + (x1-xc)*(x1-xc));
	ang1 = atan2((y1-yc)/dis,(x1-xc)/dis);
	dis = sqrt((y3-yc)*(y3-yc) + (x3-xc)*(x3-xc));
	ang2 = atan2((y3-yc)/dis,(x3-xc)/dis);
	if(ang1 < 0)ang1 += 2*PI;  if(ang2 < 0)ang2 += 2*PI;
	if (!ccw)  {
		tmp = ang1;
		ang1 = ang2;
		ang2 = tmp;
	}
	slope = tan(angle);
	incr = fabs(spacing/cos(angle));
	incr = floor(incr);
	offset = -fabs(rad/cos(angle));
	tmp = fmod(offset+yc-(xc*slope),incr);
	if (tmp < 0) offset -= tmp;
	else offset += incr-tmp;

	for (; circline(); offset += incr)  {
		dis = sqrt(ix1*ix1 + iy1*iy1);
		ang3 = atan2(iy1/dis,ix1/dis);
		dis = sqrt(ix2*ix2 + iy2*iy2);
		ang4 = atan2(iy2/dis,ix2/dis);
		if(ang3 < 0)ang3 += 2*PI; if(ang4 < 0)ang4 += 2*PI;
		if (!circ)  {
			if (INARC(ang3,ang1,ang2))  {
				if (!INARC(ang4,ang1,ang2))
					intersection(&ix2,&iy2);
			}
			else if (INARC(ang4,ang1,ang2))
					intersection(&ix1,&iy1);
				else continue;
		}
		clipseg((int)(ix1+xc),(int)(iy1+yc),(int)(ix2+xc),(int)(iy2+yc),
			NARROW,SOLID,cdptr->color);
	}
}

static
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

static
circline()
{
/*
 *  circline finds the intersections of a circle and line
 *  and returns 0 if no real intersection
 */
	double a,b,c,discr;

	a = slope * slope + 1;
	b = 2 * offset * slope;
	c = (offset*offset) - (rad*rad);
	discr = (b*b) - 4*a*c;
	if (discr < 0) return(0);
	ix1 = (-b - sqrt(discr))/(2*a);
	ix2 = (-b + sqrt(discr))/(2*a);
	iy1 = offset + ix1 * slope;
	iy2 = offset + ix2 * slope;
	return(1);
}


static
intersection(x,y)
double *x,*y;
{
/*
 *  finds intersection of fill line and flat side of arc
 */
	double s1,o1;

	if (x1 != x3)  {
		s1 = (y1-y3)/(x1-x3);
		o1 = (y1-yc) - s1 * (x1-xc);
		*x = (offset-o1)/(s1-slope);
		*y = slope * (*x) + offset;
	}
	else  {
		*x = x3-xc;
		*y = (*x) * slope + offset;
	}
}
