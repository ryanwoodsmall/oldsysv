static char SCCSID[]="@(#)gpllines.c	1.1";
/* <: t-5 d :> */
#include<stdio.h>
#include "../../../include/gpl.h"
#include "../../../include/debug.h"

gpllines(fpo,larray,larraylen,color,weight,style)
FILE *fpo;
int *larray,larraylen,color,weight,style;
{	struct command cd;

	cd.cmd = LINES;
	cd.color = color;
	cd.weight = weight;
	cd.style = style;
	cd.aptr=larray;
	cd.cnt = larraylen+2;
	putgedf(fpo,&cd);
}
