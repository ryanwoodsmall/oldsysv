/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/utodcoord.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"
extern struct window wn;
extern struct device dv;
#define XUTOD(x0)  (int)(floor(dv.v.lx+((double)x0-wn.w.lx)/wn.utodrat+.5))
#define YUTOD(y0)  (int)(floor(dv.v.ly+((double)y0-wn.w.ly)/wn.utodrat+.5))

utodcoord(cdptr)
struct command *cdptr;
{	int cnt,*lptr;
	double floor();
/*
 *  utodcoord translates GPS universe coordinates to
 *  device coordinates.  text height is also translated to device
 *  units
 */
	switch(cdptr->cmd){
	case ARCS:
	case LINES:  cnt=cdptr->cnt-2; break;
	case TEXT:  cdptr->tsiz = cdptr->tsiz/wn.utodrat;
	case ALPHA:  cnt =2; break;
	default: cnt=0;
	}
	for(lptr=cdptr->aptr;cnt>=2;cnt -= 2){
		*lptr++=XUTOD(*lptr);
		*lptr++=YUTOD(*lptr);
	}
	return(SUCCESS);
}
