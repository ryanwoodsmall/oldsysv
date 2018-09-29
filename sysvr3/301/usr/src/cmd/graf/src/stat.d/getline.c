/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/getline.c	1.6"
/* <: t-5 d :> */
#include "stdio.h"
#define ESCAPE(c,x) ( (c=x)!='\\' ? c : (x,c=x))

char *getline(fp,bufs,size) /* put next line from fp into buf */
FILE *fp;
char *bufs;
int size;
{
	char c, *bufe=bufs+size;

#if u370 | u3b | u3b5 | u3b2
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
