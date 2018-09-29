/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/ptog.d/ptogarcpt.c	1.1"
/* <: t-5 d :> */
#define SQDIF(x,y)	((x-y)*(x-y))
#define ABSVAL(x)	((x) >= 0 ? (x) : -(x))
#define PI	3.141592653589793238
arcpt(xc,yc,x1,y1,x2,y2,x,y)
	double xc,yc,x1,y1,x2,y2;
	int *x,*y; {
	double sqrt();
	double rad,rad1,rad2,r,displ;
	double orient();
	if(x1==x2 && y1==y2){
		*x=x1+2*(xc-x1);
		*y=y1+2*(yc-y1);
		return;
	}
	r = sqrt(SQDIF(xc,x1)+SQDIF(yc,y1));
	rad1 = orient(xc,yc,x1,y1);
	rad2 = orient(xc,yc,x2,y2);
	displ = ((rad2-rad1)/2);
	if (displ < 0) displ = PI + displ;
	rad = rad1 + displ;
	cirpt(xc,yc,r,rad,x,y);
	return;
}
double
orient(xc,yc,x,y)
	double xc,yc,x,y; {
	double atan2();
/*
 *  orient returns a point's orientation in radians
 *  with respect to center coordinates xc and yx.
 */
	return(atan2((y-yc),(x-xc)));
}
cirpt(xc,yc,r,rad,x,y)
	double xc,yc,r,rad;
	int *x,*y; {
	double x0,y0;
	double sin();
	double cos();
/*
 *  given orientation rad, in radians and radius, r,
 *  and enter coordinates, xc and yc, cirpt sets into x and y
 *  the values of a point on the circle with that orientation.
 */
	x0 = r*(cos(rad))+xc;
	y0 = r*(sin(rad))+yc;
	*x = ((int)x0);
	*y = ((int)y0);
	return;
}
