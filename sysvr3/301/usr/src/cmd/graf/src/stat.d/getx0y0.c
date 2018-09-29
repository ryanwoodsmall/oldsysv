/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/getx0y0.c	1.2"
#include <stdio.h>
#include "gpl.h"
#include "gsl.h"
#include "debug.h"
#include "util.h"

getx0y0(fpi,x0,y0)
FILE *fpi;
double *x0,*y0;
{	struct command cd;
	char c;
/*
 *  getx0y0 gets offset locations from file  special
 *  comment by recognizing the codeword X0Y0
 *  Kludge:  comment of length 4 is lost reguardless
 */
	if((c=getc(fpi)) != '\117'){
		ungetc(c,fpi);
		return (FAIL);
	}
	ungetc(c,fpi);
	getgedf(fpi,&cd);
	if(*(cd.aptr) != X0Y0)
		return(FAIL);
	*x0 = *(cd.aptr + 1);
	*y0 = *(cd.aptr + 2);
	return(SUCCESS);
}
