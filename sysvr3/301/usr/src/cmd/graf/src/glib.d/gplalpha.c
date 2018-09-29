/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gplalpha.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "gpl.h"
#include "debug.h"

gplalpha(fpo,larray,text)
FILE *fpo;
char *text;
int *larray;
{	struct command cd;

	cd.cmd = ALPHA;
	cd.aptr=larray;
	cd.tptr=text;
	cd.cnt = ((strlen(text)+1)/2) + 3;
	putgedf(fpo,&cd);
}
