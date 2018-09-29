/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gslswing.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#define RADIAN			57.2957795131

gslswing(xpivot,ypivot,x1,y1,ang)
double xpivot,ypivot,*x1,*y1;
int ang;
{	double sqrt(),sin(),cos();
	double cosb,sinb,r,dx,dy;
/*
 *  gslswing resets x1 and y1 to pivot on xpivot,ypivot,
 *  ang number of degrees
 */
	dx = *x1 - xpivot;
	dy = *y1 - ypivot;
	r = sqrt((dx*dx) + (dy*dy));
	if(dx==0) cosb = 0.0;
	else cosb = dx/r;
	if(dy == 0) sinb = 0.00;
	else sinb = dy/r;
	*x1 = xpivot + ((r * ((cos(ang/RADIAN) * cosb) 
	- (sin(ang/RADIAN) * sinb))) + 0.5);
	*y1 = ypivot + ((r * ((sin(ang/RADIAN) * cosb) 
	+ (cos(ang/RADIAN) * sinb))) + 0.5);
}
