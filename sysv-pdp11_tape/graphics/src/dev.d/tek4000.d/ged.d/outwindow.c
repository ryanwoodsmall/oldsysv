static char SCCSID[]="@(#)outwindow.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
extern struct control ct;
#define YFAC  (767.)

outwindow(pts,pi)
int pts[],pi;
{	struct area ar;
	double factor;
	long int lx,ly,hx,hy;
/*
 *  outwindow computes corners for windowing out
 *  and sets window parms
 */
	corners(pts,pi,&ar);
	factor = ((double)ct.w.hx - ct.w.lx)/(double)(ar.hx-ar.lx);
	lx=ct.w.lx - factor * (ar.lx-ct.w.lx);
	ly=ct.w.ly - factor * (ar.ly-ct.w.ly);

	hx=ct.w.hx + factor * (ct.w.hx-ar.hx);
	hy=ct.w.hy + factor * (ct.w.hy-ar.hy);

	sqopp(&lx,&hx);  sqopp(&ly,&hy);
	ct.w.lx=lx;  ct.w.ly=ly;
	ct.w.hx=hx;  ct.w.hy=hy;
	ct.wratio=((double)ct.w.hy-ct.w.ly)/YFAC;
	return;
}
