static char SCCSID[]="@(#)filbuf.c	1.1";
/* <:t-5 d:> */
#include "stdio.h"
#include "../../include/errpr.h"

double *filbuf(fdi,bufs,size) /*fill buf from fdi*/
FILE *fdi;
double *bufs;
int size;
{
	int s;
	double *bufe=bufs+size;

	for( ; bufs<bufe; bufs++ )
		if( getdoub(fdi,bufs)==EOF ) return(bufs-1);
	return(NULL);
}

double *filbuf1(fd,bufs,size) /* setup for filbuf */
FILE *fd;
double *bufs;
int size;
{
	double *ep, *filbuf();
	extern char *nodename;

	if((ep=filbuf(fd,bufs,size)) == NULL) {
		ERRPR1(too many data elements\, first %d used, size-1);
		ep = bufs+size-1;
	}
	return(ep);
}
