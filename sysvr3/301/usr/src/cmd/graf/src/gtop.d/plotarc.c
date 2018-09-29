/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/plotarc.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
plotarc(x0,y0,x,y,x1,y1)
	int x0,y0,x,y,x1,y1; {
	int xc,yc;
	double dxy[6];
	int param[6];

	dxy[0] = x0;	dxy[1] = y0;	/*  x0 & y0 =1st of 3 pts  */
	dxy[2] = x;	dxy[3] = y;
	dxy[4] = x1;	dxy[5] = y1;	/*  x1 & y1 =last of 3 pts  */
	arcrad(dxy,param);
	xc = param[0];
	yc = param[1];
	if(param[4]){
		arc(xc,yc,x0,y0,x1,y1);
	}
	else{
		arc(xc,yc,x1,y1,x0,y0);
	}
}
