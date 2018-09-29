/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/setspace.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "debug.h"
#include "util.h"
#include "gsl.h"
#define MINLEN     50

setspace(arptr)
struct area *arptr;
{	int deltax,deltay,xmin=0,ymin=0;
/*
 *  setspace sets a square area for the plot space cmd
 *  to have at least a MINLEN size
 */
	if((deltax=arptr->hx-arptr->lx)<MINLEN) xmin++;
	if((deltay=arptr->hy-arptr->ly)<MINLEN) ymin++;
	if(xmin && ymin) {
		extseg('=',&arptr->lx,&arptr->hx,(long)deltax-MINLEN,XYMID,XYMAX);
		extseg('=',&arptr->ly,&arptr->hy,(long)deltay-MINLEN,XYMID,XYMAX);
	}else{
		if(deltay>deltax){
			extseg('=',&arptr->lx,&arptr->hx,(long)deltay-deltax,XYMID,XYMAX);
		}
		else if (deltax>deltay){
			extseg('=',&arptr->ly,&arptr->hy,(long)deltax-deltay,XYMID,XYMAX);
		}
	}
	return(SUCCESS);
}
extseg(mode,lval,uval,amt,lbound,ubound)
char mode;
int *lval,*uval,lbound,ubound;
long int amt;
{	long delta;
/* 
 *  extseg extends a line seqment by amt units
 *  by adding or subracting amt from the proper upper or
 *  lower point.  if extention hit limits it is added to
 *  other end of segment if possible
 */
	switch (mode){
		case '-':
			if((delta=(long)*lval-lbound)>=amt){
				*lval = *lval-amt;
			}
			else{
				*lval = lbound;
				if(((long)ubound - *uval)>=amt-delta)
					*uval = *uval+(amt-delta);
				else *uval = ubound;
			}
			break;
		case '+':
			if((delta= (long)ubound-*uval)>=amt){
				*uval = *uval+amt;
			}
			else{
				*uval = ubound;
				if(((long)*lval - lbound)>=amt-delta)
					*lval= *lval-(amt-delta);
				else *lval = lbound;
			}
			break;
		case '=':
			extseg('-',lval,uval,amt/2,lbound,ubound);
			extseg('+',lval,uval,amt-(amt/2),lbound,ubound);
			break;
	}
	return;
}

