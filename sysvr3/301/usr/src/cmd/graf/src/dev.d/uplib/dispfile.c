/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/dispfile.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
extern struct window wn;

dispfile(fpi,chtable)
FILE *fpi;
char *chtable[];
{
	struct command cd;
/*
 *  dispfile gets GPS primitives, checks to see that 
 *  handles are within window, translates coordinates
 *  and displays them.
 */
	while(getgedf(fpi,&cd)!=EOF) {
		if(cd.cmd!=COMMENT && 
		  inarea(*(cd.aptr),*(cd.aptr+1),&wn.w)){
			utodcoord(&cd);
			dispprim(&cd,chtable);
		}
	}
}
