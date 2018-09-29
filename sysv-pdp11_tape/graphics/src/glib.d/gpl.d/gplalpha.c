static char SCCSID[]="@(#)gplalpha.c	1.1";
/* <: t-5 d :> */
#include<stdio.h>
#include "../../../include/gpl.h"
#include "../../../include/debug.h"

gplalpha(fpo,larray,text)
FILE *fpo;
char *text;
int *larray;
{	struct command cd;

	cd.cmd = ALPHA;
	cd.aptr=larray;
	cd.tptr=text;
	cd.cnt = ((strlen(text)+1)/2) + 3;
	putgedf(fpo,&cd);
}
