/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/glib.d/putx0y0.c	1.1"
#include <stdio.h>
#include "gsl.h"
#include "gpl.h"
#include "debug.h"

putx0y0(fpo,x0,y0)
FILE *fpo;
double x0,y0;
{	struct command cd;
/*
 *  putx0y0 puts offset locations into file as a special
 *  comment recognized by the codeword X0Y0
 */
	cd.cmd=COMMENT;  cd.cnt=4;
	cd.aptr=cd.array;
	cd.array[0]=X0Y0;
	cd.array[1]=x0;  cd.array[2]=y0;
	putgedf(fpo,&cd);
}
