/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/lolib/adjtext.c	1.3"
#include <stdio.h>
#include "dev.h"
#define PI 		3.141592653589793228

	/* generate adjacent text primitives */

adjtext(x,y,p,h,tro,chsp,wt,color,pairs,ratio)
int x,y,h,tro,wt,color,pairs,ratio;
char *p;
double chsp;
{
	double ang, sin(), cos();
	int dx, dy, i;

	ang = tro * PI/180.0;

	dx = cos(ang) * (pairs*ratio + 1);
	dy = sin(ang) * (pairs*ratio + 1);

	for (i=pairs; i>0; i--) {
		devtext(x+dx,y+dy,p,h,tro,chsp,wt,color);
		devtext(x-dx,y-dy,p,h,tro,chsp,wt,color);
		dx /= 2;  dy /= 2;
	}
}
