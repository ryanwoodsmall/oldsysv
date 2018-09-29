/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/xytek.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"

xytek(scrx,scry,tk)
int scrx,scry,tk[4];
{

	tk[0] = ((scry >> 5) & 037) + 040; /* hy */
	tk[1] =  (scry & 037) + 0140; /* ly */
	tk[2] = ((scrx >> 5) & 037) + 040; /* hx */
	tk[3] =  (scrx & 037) + 0100; /* lx */
	return;
}
