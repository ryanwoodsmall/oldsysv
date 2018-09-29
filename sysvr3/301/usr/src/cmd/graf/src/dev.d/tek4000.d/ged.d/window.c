/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/window.c	1.2"
#include <stdio.h>
#include "ged.h"

window(pts,pi,ar)
int pts[], pi;
struct area *ar;
{
	int i;

	for( i=2, ar->lx=ar->hx=pts[0], ar->ly=ar->hy=pts[1]; i<pi*2; ) {
		ar->lx = MIN(ar->lx,pts[i]);
		ar->hx = MAX(ar->hx,pts[i]); i++;
		ar->ly = MIN(ar->ly,pts[i]);
		ar->hy = MAX(ar->hy,pts[i]); i++;
	}
	if( pi<2 || (ar->lx==ar->hx && ar->ly==ar->hy) ) return(FAIL);
	else return(SUCCESS);
}
