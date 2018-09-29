static char SCCSID[]="@(#)dispfile.c	1.1";
/* <: t-5 d :> */
#include <stdio.h>
#include "../include/dev.h"
extern struct window wn;

dispfile(fpi,chtable)
FILE *fpi;
char *chtable[];
{
	struct command cd;
/*
 *  dispfile gets GPS primitives, checks to see that 
 *  handles are within window, translates coordinates
 *  and displays them.
 */
	while(getgedf(fpi,&cd)!=EOF) {
		if(cd.cmd!=COMMENT && 
		  inarea(*(cd.aptr),*(cd.aptr+1),&wn.w)){
			utodcoord(&cd);
			dispprim(&cd,chtable);
		}
	}
}
