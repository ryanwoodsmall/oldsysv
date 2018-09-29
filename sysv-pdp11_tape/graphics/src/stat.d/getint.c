static char SCCSID[]="@(#)getint.c	1.1";
/* <:t-5 d:> */
#include "stdio.h"
getint(fdi,p) /*return next integer*/
FILE *fdi;
int *p;
{
	int s;
	while((s=fscanf(fdi,"%d",p)) != EOF)
		if(s > 0) return(1);
		else if( getc(fdi)=='\1' ) { ungetc('\1',fdi); stplabel(fdi); }
	return(EOF);
}
