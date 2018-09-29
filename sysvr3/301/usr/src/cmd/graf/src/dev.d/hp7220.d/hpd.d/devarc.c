/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devarc.c	1.3"
#include <stdio.h>
#include "dev.h"
#include "hpd.h"
#define DEGPERRAD  57.2957795131
#define STDANG(ang)   if(ang<0.) ang +=360.
#define ROUND(x) (floor(x + .5))
extern int curst, curcol;
extern struct options {
	int eopt;  /*  erase  */
	int slant;  /* character slant  */
	int achset ;  /* select alternate char set  */
	int pen;  /*  select pen  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devarc(x1,y1,x2,y2,x3,y3,lw,st,color)
int x1,y1,x2,y2,x3,y3,lw,st,color;
{	int xc,yc,ccw,rad;
	double orient(),floor(),startang,endang;
	if(lw==BOLD) adjarc(x1,y1,x2,y2,x3,y3,NARROW,st,color,2,ADJRAT);
	if(lw==MEDIUM) adjarc(x1,y1,x2,y2,x3,y3,NARROW,st,color,1,ADJRAT);
	if(st != curst) curst = newst(st);
	if(op.pen == 0 && color != curcol) newcolor(color);
	MOVEPEN; mbp(x1,y1); TERMINATOR;
	arcrad(x1,y1,x2,y2,x3,y3,&xc,&yc,&rad,&ccw);
	startang = orient(xc,yc,x1,y1)*DEGPERRAD;
	endang = orient(xc,yc,x3,y3)*DEGPERRAD;
	STDANG(startang); STDANG(endang);
	if(ccw) ARCCCW; else ARCCW;
	mbn(rad);
	mba(startang); mba(endang);
	TERMINATOR;
	return(SUCCESS);
}
double
orient(xc,yc,x,y)
	int xc,yc,x,y; {
	double atan2();
/*
 *  orient returns a point's orientation in radians
 *  with respect to center coordinates xc and yx.
 */
	return(atan2(((double)y-yc),((double)x-xc)));
}
arcrad(x1,y1,x2,y2,x3,y3,xc,yc,rad,ccw)
int x1,y1,x2,y2,x3,y3,*xc,*yc,*rad,*ccw;
{	int circ;
	double r,fa,fb,fc,fd,dxc,dyc;
	double den,fxc,fyc,tmp,v1,v2;
	double dxx,dyy,sqrt(),floor();
/*
 *  arcrad computes center and radius of circle circumscribing
 *  the 3 pts; return value is 1 if arc is clockwise, else 0
 */
	circ = 0;
	fa=x2 - x1; fb=y2 - y1; 
	fc=x3 - x1; fd=y3 - y1;
	v1 = (fa*fa) + (fb*fb);
	v2 = (fc*fc) + (fd*fd);
	if(v2 == 0.0) {  /*  process full circle  */
		circ = 1;
		dxc = (x1 + x2)/2.0;
		dyc = (y1 + y2)/2.0;
		dxx = x1 -dxc;   dyy = y1 - dyc;
		r = sqrt((dxx * dxx) + (dyy * dyy));
	}
	else{
		den = 2.0 * ((fa*fd) - (fb*fc));
		fxc = ((fd * v1) - (fb * v2))/den;
		fyc = ((fa * v2) - (fc * v1))/den;
		r = sqrt((fxc * fxc) + (fyc * fyc));
		dxc = fxc + x1;
		dyc = fyc + y1;
	}
	if(!circ) {
		tmp = (x2 * fd) - (x3 * fb) - (x1 * (double)(y3 - y2));
		if(tmp < 0.0) *ccw = 0; else *ccw = 1;
	}
	*rad = ROUND(r);
	*xc=ROUND(dxc); *yc=ROUND(dyc);
	return(SUCCESS);
}
