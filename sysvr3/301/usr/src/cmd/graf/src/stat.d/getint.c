/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/getint.c	1.2"
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
