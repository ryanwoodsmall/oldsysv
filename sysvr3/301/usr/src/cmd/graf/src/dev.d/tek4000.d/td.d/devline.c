/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/td.d/devline.c	1.2"
#include <stdio.h>
#include "dev.h"

devline(x0,y0,x1,y1,wt,st,col)
int x0,y0,x1,y1,wt,st,col;
{	static int curst= -1, curwt= -1;
	int tk[4];
/*
 *  devline produces code to draw line segment from
 *  x0,y0 to x1,y1 with style st
 */
	if(wt==BOLD) adjline(x0,y0,x1,y1,MEDIUM,st,col,2,1), wt=MEDIUM;
	if( st!=curst || wt!=curwt ) newwtst(curwt=wt,curst=st);
	xytek(x0,y0,tk); printf("\035%c%c%c%c",tk[0],tk[1],tk[2],tk[3]);
	xytek(x1,y1,tk); printf("%c%c%c%c",tk[0],tk[1],tk[2],tk[3]);
}

static
newwtst(wt,st)
int wt, st;
{
/*  newwtst generates scope code to change line weight and style  
 */

	static char wtst[3][5] = { /* beam and vector selectors, table F-3 4014 manual */
		{ 'p', 'q', 'r', 's', 't' }, 
		{ '`', 'a', 'b', 'c', 'd' }, 
		{ 'h', 'i', 'j', 'k', 'l' } 
	};

	printf("\035\033%c",wtst[wt][st]);
}
