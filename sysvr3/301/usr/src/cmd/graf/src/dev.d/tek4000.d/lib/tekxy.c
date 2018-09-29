/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/tekxy.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"

tekxy(x,y,tkptr)
int *x,*y;
struct tekaddr *tkptr;
{
	*x =(((tkptr->xh & 037) << 5) + (tkptr->xl & 037));
	*y =(((tkptr->yh & 037) << 5) + (tkptr->yl & 037));
	return;
}
