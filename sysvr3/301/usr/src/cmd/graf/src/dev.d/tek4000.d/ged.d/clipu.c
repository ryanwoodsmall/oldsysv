/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/ged.d/clipu.c	1.2"
#include <stdio.h>
#include "ged.h"

clipu(x) /* clip x to universe */
long x;
{
	if( x>XYMAX ) return(XYMAX);
	else if( x<XYMIN ) return(XYMIN);
	else return(x);
}
