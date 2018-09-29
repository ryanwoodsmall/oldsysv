/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gslgsl.c	1.1"
/* <: t-5 d :> */
#include <stdio.h>
#include "debug.h"
#include "errpr.h"
#include "util.h"
#include "gsl.h"
#include "gpl.h"
#define BOUNDS(x1,y1) if(x1<XYMIN || x1>XYMAX || y1<XYMIN || y1>XYMAX){\
ERRPR2(point out of bounds %f %f,x1,y1);\
 if(x1<XYMIN) x1=XYMIN; if(x1>XYMAX) x1=XYMAX;\
 if(y1<XYMIN) y1=XYMIN; if(y1>XYMAX) y1=XYMAX;}else
#define ROUND(f)   (f>=0.) ?  f+.5 :  f-.5
static struct gslparm *Sptr;
static int array[MAXLINES * 2],*aptr,lwt=0,lst=0,lro=0,aro=0,lcol=0;
static int lflush,aflush;
extern char *nodename;

initgsl(sptr)
struct gslparm *sptr;
{
	Sptr = sptr;
	if(Sptr->xs == 0. || Sptr->ys == 0.){
		ERRPR0(scaling factors xs = 1. ys = 1.);
		Sptr->xs = Sptr->ys = 1.;
	}
	if(Sptr->fp == NULL){
		ERRPR0(output directed to stdout);
		Sptr->fp = stdout;
	}
	gplinit(Sptr->fp);
	putx0y0(Sptr->fp,Sptr->x0,Sptr->y0);
	return;
}
newparm(sptr)
struct gslparm *sptr;
{
	Sptr = sptr;
	return;
}
gslmove(x1,y1,ro)
double x1,y1;
int ro;
{
	aptr=array;  setcoord(x1,y1,ro);
	gpllines(Sptr->fp,array,(aptr-array),0,0,0);
}
gslstlines(x1,y1,ro,col,wt,st)
double x1,y1;
int ro,col,wt,st;
{
	lcol=col; lwt=wt; lst=st; lro=ro;
	lflush=0;
	aptr=array; setcoord(x1,y1,0); 
	return;
}
gslline(x1,y1)
double x1,y1;
{
	setcoord(x1,y1,lro);
	if((aptr-array) >= MAXLINES * 2){
		gslendlines();
		gslstlines(x1,y1,lro,lcol,lwt,lst);
	}
}
gslendlines()
{	int larraylen;
	if((larraylen=aptr-array)<2) return;
	if(larraylen==2 && lflush) return;
	gpllines(Sptr->fp,array,larraylen,lcol,lwt,lst);
	lflush++;
	return;
}
gslstarcs(x1,y1,ro,col,wt,st)
double x1,y1;
int ro,col,wt,st;
{
	lcol=col; lwt=wt; lst=st; aro=ro;
	aflush=0;
	aptr=array;  setcoord(x1,y1,0);
	return;
}
gslarc(x1,y1)
double x1,y1;
{
	setcoord(x1,y1,aro);
	if((aptr-array) >= MAXLINES * 2){
		gslendarcs();
		gslstarcs(x1,y1,lro,lcol,lwt,lst);
	}
}
gslendarcs()
{	int larraylen;
	if((larraylen=aptr-array)<2) return;
	if(larraylen==2 && aflush) return;
	gplarcs(Sptr->fp,array,larraylen,lcol,lwt,lst);
	aflush++;
	return;
}
gslcircle(x1,y1,rad,col,wt,st)
double x1,y1,rad;
int col,wt,st;
{
	rad *= Sptr->xs;
	aptr=array;
	setcoord(x1+rad,y1,0);
	setcoord(x1-rad,y1,0);
	setcoord(x1+rad,y1,0);
	gplarcs(Sptr->fp,array,6,col,wt,st);
	return;
}
gsltext(x1,y1,str,tro,tsz,col,font)
double x1,y1;
int tro,tsz,col,font;
char *str;
{	double ox,oy,xprint,yprint;
	int more=TRUE;
	char *ptr;
	ox = x1;	oy = y1;
	for(;more;str=ptr){
		xprint=x1; yprint=y1;
		for(ptr=str;*ptr !='\n' && *ptr!=EOS;ptr++);
		if(*ptr=='\n'){
			*ptr++=EOS;
			y1 -= 1.2*tsz;
			if(tro) gslswing(x1,y1+1.2*tsz,&x1,&y1,tro);
		}
		else more=FALSE;
		aptr=array; setcoord(xprint,yprint,0);
		gpltext(Sptr->fp,array,str,tro,tsz,col,font);
	}
	setcoord(ox,oy,0);  /*  reset cur parms  */
}
gslalpha(x1,y1,str)
double x1,y1;
char *str;
{
	aptr=array; setcoord(x1,y1,0);
	gplalpha(Sptr->fp,array,str);
	return;
}
gslcomment(larray,larraylen)
int *larray;
int larraylen;
{
	gplcomment(Sptr->fp,larray,larraylen);
	return;
}
setcoord(x1,y1,ro)
double x1,y1;
int ro;
{
	if(ro) gslswing(Sptr->ux,Sptr->uy,&x1,&y1,ro);
	BOUNDS(x1,y1);
	*aptr++=ROUND(x1); *aptr++=ROUND(y1);
	Sptr->ux=x1; Sptr->uy=y1;
	Sptr->x=x1; Sptr->y=y1;
	gslunscale(&(Sptr->x),&(Sptr->y));
	return;
}
reltoabs(x1,y1)
double *x1,*y1;
{
	*x1 += Sptr->ux; *y1 += Sptr->uy;
	return;
}
gslscale(x1,y1)
double *x1, *y1;
{
	*x1 = Sptr->x0 + *x1 * Sptr->xs;
	*y1 = Sptr->y0 + *y1 * Sptr->ys;
	return;
}
gslunscale(x1,y1)
double *x1,*y1;
{
	*x1 = (*x1 - Sptr->x0)/Sptr->xs;
	*y1 = (*y1 - Sptr->y0)/Sptr->ys;
	return;
}
inspace(x1,y1)
double x1,y1;
{
/*
 *  inspace returns SUCCESS if x1 and y1 are within
 *  boundaries of GPS universe else returns FAIL
 */
	gslscale(&x1,&y1);
	if(x1<XYMIN || x1>XYMAX || y1<XYMIN || y1>XYMAX) return(FAIL);
	else return(SUCCESS);
}
#define RADIAN     57.2957795131
xysp(xsp,ysp,ts,trot)
double *xsp,*ysp;
int ts,trot;
{	double cos(),sin();
	*xsp=(ts/14. * 11.2 * cos(trot/RADIAN));
	*ysp=(ts/14. * 11.2 * sin(trot/RADIAN));
	return;
}

gslhchar(xch,ych,tr,hsp)
double *xch,*ych,hsp;
int tr;
{	double cos(),sin();
	if(tr == 90) {
		*xch = 0.0;
		*ych = hsp;
		}
	else {
		*xch = hsp * cos(tr/RADIAN);
		*ych = hsp * sin(tr/RADIAN);
		}
}
gslvchar(xch,ych,tr,vsp)
double *xch,*ych,vsp;
int tr;
{	double cos(),sin();
	*xch = vsp * sin(tr/RADIAN);
	*ych = vsp * cos(tr/RADIAN);
	*ych *= -1;
}
gslclipu(x) /* clip x to universe */
long x;
{
	if( x>XYMAX ) return(XYMAX);
	else if( x<XYMIN ) return(XYMIN);
	else return(x);
}
gslslen(s)
char *s;
/*
 *  gslslen returns number of spaces needed to display
 *  string pointed to by s
 */
{	int i;
	for(i=0;*s != '\n' && *s != EOS; s++){
		if(*s=='\b') i--;
		else i++;
	}
	return(i);
}
