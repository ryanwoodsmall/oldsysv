/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devtext.c	1.2"
#include <stdio.h>
#include "dev.h"
#include "hpd.h"
#define ROUND(x) (floor(x + .5))
extern int curst, curcol;
extern struct options {
	int eopt;  /*  erase  */
	int slant;  /* character slant  */
	int achset ;  /* select alternate char set  */
	int pen;  /*  select pen  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devtext(x0,y0,ptr,h,ang,chsp,font,color)
int x0,y0,h,ang,font,color;
char *ptr;
double chsp;
{	static int curh = -1, curang = 0;
	double xhch,yhch,xvch,yvch,floor();
	int oang;
/*
 *  devtext produces text string with first character
 *  centered at x0,y0;
 */
	if(((font>>4)& 017) == BOLD) 
		adjtext(x0,y0,ptr,h,ang,chsp,NARROW,color,2,ADJRAT);
	if(((font>>4)& 017) == MEDIUM) 
		adjtext(x0,y0,ptr,h,ang,chsp,NARROW,color,1,ADJRAT);
	if(op.pen == 0 && color != curcol) newcolor(color);
	gslhchar(&xhch,&yhch,ang,chsp);
	gslvchar(&xvch,&yvch,ang,1.*h);
	x0 -= ROUND((xhch-xvch)/2.);
	y0 -= ROUND((yhch-yvch)/2.);
	MOVEPEN; mbp(x0,y0); TERMINATOR;
	if(h != curh){
		LABSZ; mbp((int)ROUND(chsp),(int)ROUND(2*h));
		curh = h;
	}
	if(ang != curang){
		oang = curang; curang = ang;
		ang -= oang;  /* ang relative to prev ang  */
		ang = ang % 360;
		if(ang < 0) ang += 360;
		ROTATE; mba((double)ang);
	}
	LABMODEON;
	for(; *ptr != EOS; ptr++) putchar(*ptr);
	printf("\003");
}
