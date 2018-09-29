static char SCCSID[]="@(#)pow1.c	1.1";
#include <stdio.h>
#include "../../include/errpr.h"
#include "../../include/debug.h"

double
pow1(x,y) /* set up for pow(3) */
double x,y;
{
	double fabs(), pow(), fmod();
	extern char *nodename;

	if( x<0 && fmod(y,1.)!=0 ) {
		ERRPR0(x^y with x<0 and y float yeilds -(|x|^y));
		return(-pow(fabs(x),y));
	}
	else if( x==0 && x==y) {
		return(1.0);
	}
	else return(pow(x,y));
}
