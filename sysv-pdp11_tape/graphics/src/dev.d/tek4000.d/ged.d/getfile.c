static char SCCSID[]="@(#)getfile.c	1.2";
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
/* #include "../../../../include/errpr.h" done by include ged.h */
extern char *nodename;
int
*getfile(fpi,bptr,eptr)
FILE *fpi;
int *bptr,*eptr;
{
	int chars;
	char *bcptr;
/*
 *  getfile reads file pointed to by fpi into buffer 
 *  delim by bptr & eptr
 */
	bcptr = (char *) bptr;
	while(bcptr+512 < (char *) eptr){
		if((chars=fread((char *)bcptr,sizeof(*bcptr),512,fpi))<512) {
			return((int *)(bcptr += chars));
		}
		else bcptr += chars;
	}
	ERRPR0(not enough space: input truncated);
	return((int *) bcptr);
}
