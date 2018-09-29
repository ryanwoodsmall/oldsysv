static char SCCSID[]="@(#)adjline.c	1.2";
#include <stdio.h>
#include "../include/dev.h"
#define PIby2		1.570796326794896619

	/* generate adjacent line primitives */

adjline(x0,y0,x1,y1,wt,st,color,pairs,ratio)
int x0,y0,x1,y1,wt,st,color,pairs,ratio;
{
	double ang, atan2(), sin(), cos();
	int dx, dy, i;

	ang = atan2((double)(y1-y0),(double)(x1-x0)) + PIby2;
	dx = cos(ang) * (pairs*ratio + 1);
	dy = sin(ang) * (pairs*ratio + 1);
	for (i=pairs; i>0; i--) {
		clipseg(x0-dx,y0-dy,x1-dx,y1-dy,wt,st,color);
		clipseg(x0+dx,y0+dy,x1+dx,y1+dy,wt,st,color);
		dx /= 2;  dy /= 2;
	}
}
