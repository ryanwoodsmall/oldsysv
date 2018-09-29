/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gsl.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "debug.h"
#include "util.h"
#include "gsl.h"

move(x1,y1,ro)
double x1,y1;
int ro;
{
	gslscale(&x1,&y1);
	gslmove(x1,y1,ro);
}
lines(larray,larraylen,ro,col,wt,st)
double *larray;
int larraylen,ro,col,wt,st;
{
	if(larraylen<2) return;
	startlines(*larray,*(larray+1),ro,col,wt,st);
	for(larray+=2,larraylen-=2; larraylen>0; larray+= 2,larraylen-=2){
		line(*larray,*(larray+1));
	}
	endlines();
	return;
}
startlines(x1,y1,ro,col,wt,st)
double x1,y1;
int ro,col,wt,st;
{
	gslscale(&x1,&y1);
	gslstlines(x1,y1,ro,col,wt,st);
}
line(x1,y1)
double x1,y1;
{
	gslscale(&x1,&y1);
	gslline(x1,y1);
}
endlines()
{
	gslendlines();
}
circle(x1,y1,rad,col,wt,st)
double x1,y1,rad;
int col,wt,st;
{
	gslscale(&x1,&y1);
	gslcircle(x1,y1,rad,col,wt,st);
}
arcs(larray,larraylen,ro,col,wt,st)
double *larray;
int larraylen,ro,col,wt,st;
{
	if(larraylen <2) return;
	startarcs(*larray,*(larray+1),ro,col,wt,st);
	for(larraylen -= 2; larraylen>0;  larray +=2,larraylen -=2){
		arc(*larray,*(larray+1));
	}
	endarcs();
	return;
}
startarcs(x1,y1,ro,col,wt,st)
double x1,y1;
int ro,col,wt,st;
{
	gslscale(&x1,&y1);
	gslstarcs(x1,y1,ro,col,wt,st);
}
arc(x1,y1)
double x1,y1;
{
	gslscale(&x1,&y1);
	gslarc(x1,y1);
}
endarcs()
{
	gslendarcs();
}
text(x1,y1,str,tro,tsz,col,font)
double x1,y1;
int tro,tsz,col,font;
char *str;
{
	gslscale(&x1,&y1);
	gsltext(x1,y1,str,tro,tsz,col,font);
}
alpha(x1,y1,str)
double x1,y1;
char *str;
{
	gslscale(&x1,&y1);
	gslalpha(x1,y1,str);
}
comment(array,arraylen)
int *array;
int arraylen;
{
	gslcomment(array,arraylen);
}
