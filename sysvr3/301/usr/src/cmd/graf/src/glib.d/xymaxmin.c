/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/xymaxmin.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "gsl.h"
#include "gpl.h"
#include "util.h"
#include "debug.h"


/*
The entries in this table represent combinations
of the 4 points on the square below.
1 represents pt1, 2 pt2, 4 pt3, & 8 pt4.
Sums represent combinations: 6 is pts 2 & 3.
The square circumscribes a circle which is 
coincident with an arc.
We are trying to find the max & min values of 
x and y over the arc.

		pt1
	_________________
	|		|
	|		|
	|		|
	|		|
    pt4 |		|pt2
	|		|
	|		|
	|		|
	|		|
	_________________
		pt3

The index into the table is made from the quadrant 
numbers of the 3 points defining the arc.
Each arc is in a 2 bit quadrant.
(Numbered 0 to 3 instead of 1 to 4.)
Concatenating the 2 bits for the 3 arcs gives
a 6 bit index into 64 entries.

Some entries (5,10,11,13,14) are not possible.
Some combinations of points in quadrants cannot
be a valid arc.
The entry 13 is used to indicate this.
The entries are packed 2 to a char.
	*/

#define ILLEGAL 13
#define PACK(a,b) ((a) << 4) | (b)

unsigned char quadtab[] = {
	PACK(0,1),
	PACK(13,2),
	PACK(15,1),
	PACK(9,13),
	PACK(13,13),
	PACK(13,13),
	PACK(13,13),
	PACK(6,2),
	PACK(1,13),
	PACK(13,3),
	PACK(1,0),
	PACK(8,13),
	PACK(13,13),
	PACK(8,12),
	PACK(13,13),
	PACK(13,13),
	PACK(13,13),
	PACK(13,13),
	PACK(9,8),
	PACK(13,13),
	PACK(13,8),
	PACK(0,4),
	PACK(6,13),
	PACK(13,4),
	PACK(2,3),
	PACK(13,13),
	PACK(13,13),
	PACK(13,13),
	PACK(13,12),
	PACK(4,13),
	PACK(2,13),
	PACK(4,0)
};


xymaxmin(fpi,fpo,arptr)
FILE *fpi,*fpo;
struct area *arptr;
{	int cnt,*lptr,sz=0,init=TRUE;
	struct command cd;
	double dxy[14], fourpts[8];
	int param[6], i, quad, qindex, idxy, fourptsflag;


/*
 *  xymaxmin reads GPS from fpi and copies to fpo and
 *  sets max and min coords into struct pointed to by arptr
 *  & returns max text size
 */
	while(getgedf(fpi,&cd) != EOF){
		if(init && cd.cmd != COMMENT){
			arptr->lx = arptr->hx = *cd.aptr;
			arptr->ly = arptr->hy = *(cd.aptr+1);
			init=FALSE;
		}
		switch(cd.cmd){
			case ARCS: if(cd.cnt == 8) {	/* not lines; else drop to next case */
				for(lptr=cd.aptr,cnt=1; cnt<=6; cnt+=2,lptr+=2) {
					dxy[cnt - 1] = *lptr;
					dxy[cnt] = *(lptr + 1);
					}
				/* 
				radarc returns with:
					param[0] = x coord of center
					param[1] = y coord of center
					param[2] = radius of arc
					param[3] != 0 if circle
				*/

				radarc(dxy, param);

				/*
				Compute the 4 extreme points of the circle 
				coincident with the arc.
				The four points are, in order,
					top, rt, bottom, left.
				*/
				fourpts[0] = param[0];
				fourpts[1] = param[1] + param[2];
				fourpts[2] = param[0] + param[2];
				fourpts[3] = param[1];
				fourpts[4] = param[0];
				fourpts[5] = param[1] - param[2];
				fourpts[6] = param[0] - param[2];
				fourpts[7] = param[1];

				/*
				Compute the quadrant each point of the arc is 
				located in.
				Note that quadrants are numbered from 0 to 3
				instead of the usual 1 to 4.
				These numbers are shifted and added to give
				an index into table quadtab.
				This table provides a flag word that indicates
				which of the four points are to be used in
				computing the max and min.
				*/
				qindex = 0;
				for(i = 0; i <= 5; i += 2) {
					if(dxy[i] > 0.0)
						if(dxy[i+1] > 0.0) quad = 0; /* +,+ */
						else quad = 3;
					else
						if(dxy[i+1] > 0.0) quad = 1; /* -,+ */
						else quad = 2;
					qindex = (qindex << 2) + quad;
					}

				if(param[3]) fourptsflag = 15;	/* circle */
				else {	/* get and unpack */
					fourptsflag = quadtab[qindex/2];
					if(qindex & 1) fourptsflag = fourptsflag & 0xf;
					else fourptsflag >>= 4;
					}

				idxy = 5;
				for(i=0; i<=7; i+=2) {
					if(fourptsflag & 1) {
						++idxy;
						dxy[idxy] = fourpts[i];
						++idxy;
						dxy[idxy] = fourpts[i+1];
						}
					fourptsflag >>= 1;
					}

				/* find max/min x/y */

				for(i = 0; i <= idxy; i += 2) {
					arptr->lx = MIN(arptr->lx, (int) gslclipu((long) dxy[i]));
					arptr->ly = MIN(arptr->ly, (int) gslclipu((long) dxy[i+1]));
					arptr->hx = MAX(arptr->hx, (int) gslclipu((long) dxy[i]));
					arptr->hy = MAX(arptr->hy, (int) gslclipu((long) dxy[i+1]));
					}
				break;
				}
			case LINES:
			case ALPHA:
				for(lptr=cd.aptr,cnt=cd.cnt-2;cnt>0;cnt-=2,lptr+=2){
					arptr->lx = MIN(arptr->lx,*lptr); 
	   				arptr->ly = MIN(arptr->ly,*(lptr+1)); 
	   				arptr->hx = MAX(arptr->hx,*lptr); 
	   				arptr->hy = MAX(arptr->hy,*(lptr+1)) ;
				}
				break;
			case TEXT:  textmaxmin(&cd,arptr); 
					sz=MAX(sz,cd.tsiz);
					break;
		}
		if(fpo != NULL) putgedf(fpo,&cd);
	}
	return(sz);
}
#define EXPAND(x1) ((x1>0)? x1+1 : x1 -1)

textmaxmin(cdptr,arptr)
struct command *cdptr;
struct area *arptr;
{	double x1,y1,x2,y2;
	double xhch,yhch,xvch,yvch,floor();
/*
 *  textmaxmin computes max and min for the text string
 *  in cdptr and compares with hi-lo values in arptr
 */
	gslhchar(&xhch,&yhch,cdptr->trot,.8*cdptr->tsiz);
	gslvchar(&xvch,&yvch,cdptr->trot,1.2*cdptr->tsiz);
	x1 = *cdptr->aptr; y1 = *(cdptr->aptr+1);
	x1 += xvch; y1 += yvch;  /*  decend full line to allow for descending chars  */
	x1 -= xhch/2.; y1 -= yhch/2.;  /*  to low left corner  */

	x2 = x1 + (gslslen(cdptr->tptr)) * xhch;
	y2 = y1 + (gslslen(cdptr->tptr)) * yhch;

	test(arptr,x1,y1);  test(arptr,x2,y2);
	x1 -= xvch*1.5; y1 -= yvch*1.5;
	x2 -= xvch*1.5; y2 -= yvch*1.5;
	test(arptr,x1,y1); test(arptr,x2,y2);
}
static
test(arptr,x,y)
struct area *arptr;
double x,y;
{
	x=EXPAND(x); y=EXPAND(y);
	arptr->lx = MIN(arptr->lx,(int)gslclipu((long)x));
	arptr->ly = MIN(arptr->ly,(int)gslclipu((long)y));
	arptr->hx = MAX(arptr->hx,(int)gslclipu((long)x));
	arptr->hy = MAX(arptr->hy,(int)gslclipu((long)y));
}
	/*
	Pulled from /src/gutil.d/gtop.dp.
	The routine was called arcrad.  The name was changed
	avoid mult def errors from ld.
	Not all of this is needed here.
	*/

radarc(dxy,param)
double dxy[6];  int param[6];
{

int circ, ccw;
double r,xc,yc,fa,fb,fc,fd;
double den,fxc,fyc,tmp,v1,v2;
double x1,y1,x2,y2,x3,y3,dxx,dyy;
double sqrt();

circ = 0;
ccw = 1;
x1 = dxy[0];  y1 = dxy[1];
x2 = dxy[2];  y2 = dxy[3];
x3 = dxy[4];  y3 = dxy[5];
fa = x2 - x1; fb = y2 - y1; fc = x3 - x1; fd = y3 - y1;
if ( ( fb == 0 ) && ( fd == 0 ) ) { /* When coordinates are rounded to integers */
	if ( y2 > 0 ) { fb++; }     /* small differences are lost and points form */
	else { fb--; }		    /* a straight line.  This causes a divide by */
}				    /* zero.  Test to make sure that middle coord. */
else {				    /* always differs by at least one */
	if ( ( fa == 0 ) && ( fc == 0 ) ) {
		if ( x2 > 0 ) { fa++; }
		else { fa--; }
	}
}
v1 = (fa*fa) + (fb*fb);
v2 = (fc*fc) + (fd*fd);
/* process full circle   */
if(v2 == 0.0) {
	circ = 1;
	xc = (x1 + x2)/2.0;
	yc = (y1 + y2)/2.0;
	dxx = x1 -xc;   dyy = y1 - yc;
	r = sqrt((dxx * dxx) + (dyy * dyy));
	}
  
else  {
	den = 2.0 * ((fa*fd) - (fb*fc));
	fxc = ((fd * v1) - (fb * v2))/den;
	fyc = ((fa * v2) - (fc * v1))/den;
	r = sqrt((fxc * fxc) + (fyc * fyc));
	xc = fxc + x1;
	yc = fyc + y1;
	}
/* if tmp is > 0 arc is ccw if < arc is cw  */
if(circ == 0) {
	tmp = (x2 * fd)- (x3 *fb) - (x1 * (y3 -y2));
	if(tmp < 0.0) ccw = 0;
	}
param[0] = xc;  param[1] = yc;
param[2] = r; param[3] = circ;
param[4] = ccw;
return(0);
}
