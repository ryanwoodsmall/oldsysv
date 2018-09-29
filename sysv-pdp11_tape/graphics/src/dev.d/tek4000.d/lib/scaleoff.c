static char SCCSID[]="@(#)scaleoff.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
extern struct control ct;

scaleoff(x,y)
int *x,*y;
{	double dx,dy;
/*  scaleoff translates x and y coordinates from
 *  screen space(767 by 1023) to window space in ged universe
 *  if out of universe coordinate is set on boundary
 */
	dx = (ct.wratio * *x) + ct.w.lx;
	dy = (ct.wratio * *y) + ct.w.ly;
	if(dx>XYMAX)dx=XYMAX;  if(dx<XYMIN)dx=XYMIN;
	if(dy>XYMAX) dy=XYMAX;  if(dy<XYMIN)dy=XYMIN;
	*x=dx;  *y=dy;
	return;
}

