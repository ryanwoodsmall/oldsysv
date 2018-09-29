/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/compwind.c	1.2"
#include <stdio.h>
#include "dev.h"
#define MINWIN	(5.)

compwind(wp,dv)
struct window *wp;
struct device dv;
{
	corners(&wp->w,dv);
	wp->utodrat = ((double)wp->w.hy-wp->w.ly)/(dv.v.hy-dv.v.ly);
	return(SUCCESS);
}
static
corners(arptr,dv)
struct area *arptr;
struct device dv;
{	double deltax,deltay,xrat,yrat,xdv,ydv;
	long lx,ly,hx,hy,amt;
/*
 *  corners computes window corners from values in window structure
 */
	lx=arptr->lx; ly=arptr->ly; hx=arptr->hx; hy=arptr->hy;
	deltax = hx-lx;  deltay = hy-ly;
	if(deltax<MINWIN && deltay<MINWIN){
		deltax= MINWIN;	deltay = MINWIN;
	}
	xdv = dv.v.hx - dv.v.lx;  ydv = dv.v.hy - dv.v.ly;
	if((xrat=deltax/xdv)>(yrat=deltay/ydv)){  /*  extend y */
		ly -= (amt=(((ydv * xrat)-(deltay))/2. +.5));
		hy += amt;
	}
	else{		/*  extend x  */
		lx -= (amt=(((xdv * yrat)-(deltax))/2. +.5));
		hx += amt;
	}
	sqopp(&lx,&hx);  sqopp(&ly,&hy);
	arptr->hx =hx; arptr->hy=hy;  arptr->lx =lx; arptr->ly=ly;
	return(SUCCESS);
}
static
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
