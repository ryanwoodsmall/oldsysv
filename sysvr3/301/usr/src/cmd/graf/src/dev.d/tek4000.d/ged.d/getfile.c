/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/getfile.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "ged.h"
/* #include "errpr.h" done by include ged.h */
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
