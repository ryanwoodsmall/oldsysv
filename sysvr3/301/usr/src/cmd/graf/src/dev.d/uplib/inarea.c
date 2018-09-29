/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/uplib/inarea.c	1.2"
/* <: t-5 d :> */
#include <stdio.h>
#include "dev.h"

inarea(x,y,arptr)
int x,y;
struct area *arptr;
{
/*
 *  inarea returns SUCCESS if x and y are within
 *  the area defined by arptr
 */
	if(x >=arptr->lx && x <= arptr->hx && 
	 y >= arptr->ly && y <= arptr->hy) return(SUCCESS);
	else return(FAIL);
}
