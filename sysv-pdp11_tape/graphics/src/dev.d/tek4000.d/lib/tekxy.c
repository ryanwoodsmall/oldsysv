static char SCCSID[]="@(#)tekxy.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"

tekxy(x,y,tkptr)
int *x,*y;
struct tekaddr *tkptr;
{
	*x =(((tkptr->xh & 037) << 5) + (tkptr->xl & 037));
	*y =(((tkptr->yh & 037) << 5) + (tkptr->yl & 037));
	return;
}
