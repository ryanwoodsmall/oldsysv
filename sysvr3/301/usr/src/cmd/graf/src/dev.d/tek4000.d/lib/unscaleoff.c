/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/lib/unscaleoff.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "tek.h"
extern struct control ct;

unscaleoff(x,y)
int *x,*y;
{
/*  unscaleoff translates x and y coordinates from window space
 *  in ged universe to screen space
 */
	*x = ((*x-(double)ct.w.lx)/ct.wratio);
	*y = ((*y-(double)ct.w.ly)/ct.wratio);
	return;
}
