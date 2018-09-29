/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/lolib/adjarc.c	1.2"
#include <stdio.h>
#include "dev.h"

	/* generate adjacent arc primitives */

adjarc(x0,y0,x1,y1,x2,y2,wt,st,color,pairs,ratio)
int x0,y0,x1,y1,x2,y2,wt,st,color,pairs,ratio;
{
	double fa, fb, fc, fd, dxc, dyc;
	double den, fxc, fyc;
	double v1, v2, ang;
	double floor(), atan2(), sin(), cos();
	int xc, yc;
	int dx[3], dy[3], x[3], y[3];
	int i, j;

	fa = x1 - x0;  fb = y1 - y0;
	fc = x2 - x0;  fd = y2 - y0;
	v1 = (fa*fa) + (fb*fb);
	v2 = (fc*fc) + (fd*fd);
	if (v2 == 0.0) {
		dxc = (x0 + x1)/2.0;
		dyc = (y0 + y1)/2.0;
	}
	else {
		den = 2.0 * ((fa*fd) - (fb*fc));
		fxc = ((fd*v1) - (fb*v2))/den;
		fyc = ((fa*v2) - (fc*v1))/den;
		dxc = fxc + x0;
		dyc = fyc + y0;
	}
	xc = floor(dxc + 0.5);
	yc = floor(dyc + 0.5);

	x[0] = x0;  y[0] = y0;
	x[1] = x1;  y[1] = y1;
	x[2] = x2;  y[2] = y2;

	for (i=0; i<3; i++) {
		ang = atan2((double)(y[i]-yc),(double)(x[i]-xc));
		dx[i] = cos(ang) * (pairs*ratio + 1);
		dy[i] = sin(ang) * (pairs*ratio + 1);
	}
	for (i=pairs; i>0; i--) {
		devarc(x0-dx[0],y0-dy[0],x1-dx[1],y1-dy[1],x2-dx[2],
			y2-dy[2],wt,st,color);
		devarc(x0+dx[0],y0+dy[0],x1+dx[1],y1+dy[1],x2+dx[2],
			y2+dy[2],wt,st,color);
		for (j=0; j<3; j++) {
			dx[j] /= 2;  dy[j] /= 2;
		}
	}
}
