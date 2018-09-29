/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/setwindow.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
#define XFAC	(1023.)
#define YFAC	(767.)
#define MINWIN	(100.)
extern struct control ct;

setwindow(pts,pi)
int pts[],pi;
{
	if(!corners(pts,pi,&ct.w)) return(FAIL);
	ct.wratio = ((double)ct.w.hy-ct.w.ly)/YFAC;
	return(SUCCESS);
}
corners(pts,pi,arptr)
int pts[], pi;
struct area *arptr;
{	double deltax,deltay,xrat,yrat;
	long lx,ly,hx,hy,amt;
/*
 *  corners computes window corners from pts array
 *  and stores in arptr
 */
	if(!window(pts,pi,arptr)) return(FAIL);
	lx=arptr->lx; ly=arptr->ly;
	hx=arptr->hx; hy=arptr->hy;
	deltax = hx-lx;  deltay = hy-ly;
	if( deltax<MINWIN && deltay<MINWIN ) {
		deltax = MINWIN; deltay = MINWIN;
		hx = lx+deltax; hy = ly+deltay;
	}
	if((xrat=deltax/XFAC)>(yrat=deltay/YFAC)){  /*  extend y */
		ly -= (amt=(((YFAC * xrat)-(deltay))/2. +.5));
		hy += amt;
	}
	else{		/*  extend x  */
		lx -= (amt=(((XFAC * yrat)-(deltax))/2. +.5));
		hx += amt;
	}
	sqopp(&lx,&hx);  sqopp(&ly,&hy);
	arptr->hx =hx; arptr->hy=hy;
	arptr->lx =lx; arptr->ly=ly;
	return(SUCCESS);
}
sqopp(l,h)
long int *l,*h;
{
/*
 *  sqopp squishes opposite sides to fit in universe
 */
	if(*l<XYMIN){
		if((*h += (XYMIN-*l)) > XYMAX)*h=XYMAX;
		*l=XYMIN;
	}
	if(*h>XYMAX){
		if((*l -= (*h-XYMAX)) < XYMIN) *l=XYMIN;
		*h=XYMAX;
	}
	return;
}
