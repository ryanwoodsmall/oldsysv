static char SCCSID[]="@(#)unscaleoff.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
extern struct control ct;

unscaleoff(x,y)
int *x,*y;
{
/*  unscaleoff translates x and y coordinates from window space
 *  in ged universe to screen space
 */
	*x = ((*x-(double)ct.w.lx)/ct.wratio);
	*y = ((*y-(double)ct.w.ly)/ct.wratio);
	return;
}
