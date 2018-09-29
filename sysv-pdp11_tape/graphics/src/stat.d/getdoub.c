static char SCCSID[]="@(#)getdoub.c	1.1";
/* <:t-5 d:> */
#include "stdio.h"
getdoub(fdi,p) /* return next double in place pointed by p */
FILE *fdi;
double *p;
{
	int s;

	while((s=fscanf(fdi,"%F",p)) != EOF)
		if(s > 0) return(1);
		else if(getc(fdi)=='\1') { ungetc('\1',fdi); stplabel(fdi); }
	return(EOF);
}
