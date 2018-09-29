static char SCCSID[]="@(#)getlabel.c	1.1";
/* <:t-5 d:> */
#include "stdio.h"
char *getlabel(fdi,bufs,size) /* get label (if it exists) from input */
FILE *fdi;
char *bufs;
int size;
{
	int c;
	char *getline();

	if((c=getc(fdi))=='\1' )
		return( getline(fdi,bufs,size) );
	else {
		ungetc(c,fdi);
		*bufs = '\0';
		return(0);
	}
}

stplabel(fdi) /* strip label */
FILE *fdi;
{
	char cp[3];

	while( getlabel(fdi,cp,3) > cp ) ;
}
