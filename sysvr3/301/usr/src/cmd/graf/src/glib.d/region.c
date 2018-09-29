/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/region.c	1.1"
#include <stdio.h>
#include "util.h"
#include "debug.h"
#include "gsl.h"

#define NOREGIONS    5
int regtab[5]= {XYMIN,XYMIN+XDIM,XYMIN+2*XDIM,XYMAX-2*XDIM,XYMAX-XDIM};

region(reg,lx,ly)
int reg;
double *lx,*ly;
{
/*
 *  given an integer between 0-15 region returns the
 *  coordinates of the lower left corner of the region
 *  if an unallowable region is given R10 is returned
 */
	if(reg <1 || reg>25) reg=13;
	reg--;
	*lx = regtab[reg%NOREGIONS];
	*ly = regtab[reg/NOREGIONS];
	return(SUCCESS);
}
