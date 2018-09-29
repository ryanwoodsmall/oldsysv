/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/gplinit.c	1.1"
/* <: t-5 d :> */
#include<stdio.h>
#include "gpl.h"
#include "util.h"
gplinit(fpo)
FILE *fpo; 
{	int i[];
	gplcomment(fpo,i,0);
}
