static char SCCSID[]="@(#)region.c	1.1";
#include <stdio.h>
#include "../../../include/util.h"
#include "../../../include/debug.h"
#include "../../../include/gsl.h"

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
