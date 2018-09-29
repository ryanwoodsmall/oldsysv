static char SCCSID[]="@(#)tekarc.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
#include "../../../../include/util.h"

tekarc(cdptr)
struct command *cdptr;
{	int pts;
/*
 *  tekarc build tek scope code to produce arc from the
 *  location cdptr->x1,cdptr->y1 through cdptr->x2,cdptr->y2 
 *  and ending at cdptr->x3,cdptr->y3
 */
	if((pts=((cdptr->cnt-2)/2)) <= 0) return(FAIL);
	switch(pts){
	case 1: case 2:  teklines(cdptr); break;
	case 3:  circarc(cdptr); break;
	default: teklines(cdptr);  /*  spline later  */
	}
	return(SUCCESS);
}
