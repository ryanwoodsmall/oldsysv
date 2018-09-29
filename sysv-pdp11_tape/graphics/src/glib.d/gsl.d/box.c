static char SCCSID[]="@(#)box.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "../../../include/debug.h"

box(x1,y1,x2,y2,ro,color,weight,style) /* draw box from x1,y1 to x2,y2 */
double x1,y1,x2,y2;
int ro,color,weight,style;
{
	gslscale(&x1,&y1);
	gslscale(&x2,&y2);
	gslbox(x1,y1,x2,y2,ro,color,weight,style);
}

gslbox(x1,y1,x2,y2,ro,color,weight,style)
double x1,y1,x2,y2;
int ro,color,weight,style;
{
	gslstlines(x1,y1,ro,color,weight,style);
	gslline(x2,y1);
	gslline(x2,y2);
	gslline(x1,y2);
	gslline(x1,y1);
	gslendlines();
}
