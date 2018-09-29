/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gpllines.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "gpl.h"
#include "debug.h"

gpllines(fpo,larray,larraylen,color,weight,style)
FILE *fpo;
int *larray,larraylen,color,weight,style;
{	struct command cd;

	cd.cmd = LINES;
	cd.color = color;
	cd.weight = weight;
	cd.style = style;
	cd.aptr=larray;
	cd.cnt = larraylen+2;
	putgedf(fpo,&cd);
}
