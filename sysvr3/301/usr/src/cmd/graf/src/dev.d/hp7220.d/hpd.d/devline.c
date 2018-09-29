/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/devline.c	1.2"
#include <stdio.h>
#include "dev.h"
#include "hpd.h"
extern int curst,curcol;
extern struct options {
	int eopt;  /*  erase  */
	int slant;  /* character slant  */
	int achset ;  /* select alternate char set  */
	int pen;  /*  select pen  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devline(x0,y0,x1,y1,wt,st,color)
int x0,y0,x1,y1,wt,st,color;
{
/*
 *  devline produces code to draw line segment from
 *  x0,y0 to x1,y1 with style st
 */
	if(wt==BOLD) adjline(x0,y0,x1,y1,NARROW,st,color,2,ADJRAT);
	if(wt==MEDIUM) adjline(x0,y0,x1,y1,NARROW,st,color,1,ADJRAT);
	if(st != curst) curst=newst(st);
	if( op.pen==0 && color != curcol) newcolor(color);
	MOVEPEN; mbp(x0,y0); TERMINATOR;
	DRAW; mbp(x1,y1); TERMINATOR;
}

newst(st)
int st;
{
	switch(st){
	case SOLID: 	FIXDASHL; break;
	case DOTTED:   FIXDASHL;
				dashlen(1); spacelen(4);
				dashlen(2); spacelen(4);
				dashlen(1); mbn(160);
				break;
	case DASHED:   FIXDASHL;
				dashlen(1); spacelen(1);
				dashlen(2); spacelen(1);
				dashlen(1); mbn(280);
				break;
	case LONGDASH:   FIXDASHL;
				dashlen(1); spacelen(1);
				dashlen(2); spacelen(1);
				dashlen(1); mbn(440);
				break;
	case DOTDASH:   FIXDASHL;
				dashlen(3); spacelen(2);
				dashlen(1); spacelen(2);
				dashlen(6); spacelen(2);
				dashlen(1); spacelen(2);
				dashlen(3); mbn(640);
				break;
	default:		FIXDASHL; return(SOLID);
	}
	return(st);
}
static dashlen(i){
	putchar((1<<5)+i);
}
static spacelen(i){
	putchar((1<<6)+i);
}
