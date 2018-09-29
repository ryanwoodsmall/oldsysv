static char SCCSID[]="@(#)gplcomment.c	1.1";
/* <: t-5 d :> */
#include<stdio.h>
#include "../../../include/gpl.h"
#include "../../../include/debug.h"

gplcomment(fpo,larray,larraylen)
FILE *fpo;
int larray[],larraylen;
{	struct command cd;

	cd.cmd = COMMENT;
	cd.aptr=larray;
	cd.cnt = larraylen+1;
	putgedf(fpo,&cd);
}
