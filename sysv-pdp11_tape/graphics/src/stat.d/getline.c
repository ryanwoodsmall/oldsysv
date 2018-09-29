static char SCCSID[]="@(#)getline.c	1.3";
/* <: t-5 d :> */
#include "stdio.h"
#define ESCAPE(c,x) ( (c=x)!='\\' ? c : (x,c=x))

char *getline(fp,bufs,size) /* put next line from fp into buf */
FILE *fp;
char *bufs;
int size;
{
	char c, *bufe=bufs+size;

#if u370 | u3b
	while( ESCAPE(c,getc(fp))!=(char)EOF )
#else
	while( ESCAPE(c,getc(fp))!=EOF )
#endif
		if( c!='\n' ) {
			if( bufs<bufe-1 ) *bufs++ = c;
		} else {
			*bufs = '\0';
			return(bufs);
		}
	return((char *) EOF);
}
