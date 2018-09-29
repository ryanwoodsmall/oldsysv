static char SCCSID[]="@(#)gpltext.c	1.1";
/* <: t-5 d :> */
#include<stdio.h>
#include "../../../include/gpl.h"
#include "../../../include/debug.h"

gpltext(fpo,larray,text,trot,tsiz,color,font)
FILE *fpo;
char *text;
int *larray,trot,tsiz,color,font;
{	struct command cd;

	cd.cmd = TEXT;
	cd.trot = trot;
	cd.tsiz = tsiz;
	cd.color = color;
	cd.font = font;
	cd.aptr=larray;
	cd.tptr=text;
	cd.cnt = ((strlen(text)+2)/2) + 5;
	putgedf(fpo,&cd);
}
