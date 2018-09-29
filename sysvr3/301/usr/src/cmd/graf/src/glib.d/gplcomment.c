/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gplcomment.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "gpl.h"
#include "debug.h"

gplcomment(fpo,larray,larraylen)
FILE *fpo;
int larray[],larraylen;
{	struct command cd;

	cd.cmd = COMMENT;
	cd.aptr=larray;
	cd.cnt = larraylen+1;
	putgedf(fpo,&cd);
}
