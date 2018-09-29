static char SCCSID[]="@(#)getx0y0.c	1.1";
#include <stdio.h>
#include "../../include/gpl.h"
#include "../../include/gsl.h"
#include "../../include/debug.h"
#include "../../include/util.h"

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
