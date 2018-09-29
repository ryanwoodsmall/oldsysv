/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/xymxmn.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
#define MARGRAT (.10)
#define MINMARG (10)


xymxmn(bptr,endbuf,arptr)
int *bptr,*endbuf;
struct area *arptr;
{
	int cnt, *lptr, marg, *getgeds();
	struct command cd;
/*
 *  xymxmn reads GPS from buffer pointed to by bptr
 *  and sets max and min coords into struct pointed to by arptr
 */
	arptr->lx = XYMAX;	arptr->ly = XYMAX;
	arptr->hx = XYMIN;	arptr->hy = XYMIN;
	while((bptr=getgeds(bptr,endbuf,&cd)) != NULL){
		if(cd.cmd != COMMENT) {
			if(cd.cmd==LINES || cd.cmd==ARCS)cnt=cd.cnt-2;
			else cnt=2;
			for(lptr=cd.aptr;cnt>0;cnt-=2,lptr+=2){
				arptr->lx = MIN(arptr->lx,*lptr); 
		   		arptr->ly = MIN(arptr->ly,*(lptr+1)); 
		   		arptr->hx = MAX(arptr->hx,*lptr); 
		   		arptr->hy = MAX(arptr->hy,*(lptr+1));
			}
		}
	}
	marg= MAX((long)arptr->hx-arptr->lx,arptr->hy-arptr->ly)*MARGRAT;
	marg=MAX(marg,MINMARG);
	arptr->lx = clipu(arptr->lx -(long)marg);
	arptr->ly = clipu(arptr->ly -(long)marg);
	arptr->hx = clipu(arptr->hx +(long)marg);
	arptr->hy = clipu(arptr->hy +(long)marg);
	return(SUCCESS);
}
