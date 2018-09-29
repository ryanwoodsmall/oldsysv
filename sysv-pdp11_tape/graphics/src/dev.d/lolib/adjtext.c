static char SCCSID[]="@(#)adjtext.c	1.2";
#include <stdio.h>
#include "../include/dev.h"
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
