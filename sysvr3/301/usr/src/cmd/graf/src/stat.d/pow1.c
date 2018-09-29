/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/pow1.c	1.3"
#include <stdio.h>
#include "errpr.h"
#include "debug.h"

double
pow1(x,y) /* set up for pow(3) */
double x,y;
{
	double fabs(), pow(), fmod();
	extern char *nodename;

	if( x<0 && fmod(y,1.)!=0 ) {
		ERRPR0(x^y with x<0 and y float yields -(|x|^y));
		return(-pow(fabs(x),y));
	}
	else if( x==0 && x==y) {
		return(1.0);
	}
	else return(pow(x,y));
}
