/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/getline.c	1.2"
/* <: t-5 d :> */
#include "stdio.h"

char *getline(fp,bufs,size) /* put next line from fp into buf */
FILE *fp;
char *bufs;
int size;
{
	int c;
	char *bufe=bufs+size;

	while( (c=getc(fp))!=EOF ) {
		if( c=='\\' )
			switch( c=getc(fp) ) {
			case 'b': c='\b'; break;
			case 'f': c='\f'; break;
			case 'n': c='\n'; break;
			case 't': c='\t'; break;
			case '\n': c='\n'; break;
			case EOF: return(NULL);
			default: break;
			}
		else if( c=='\n' ) {
			*bufs = '\0';
			return(bufs);
		}
		if( bufs<bufe-1 ) *bufs++ = c;
	}
	return(NULL);
}
