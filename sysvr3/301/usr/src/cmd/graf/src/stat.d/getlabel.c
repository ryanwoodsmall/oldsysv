/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/getlabel.c	1.2"
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
