/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/gtop.d/arcrad.c	1.1"
/* <: t-5 d :> */
arcrad(dxy,param)
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
