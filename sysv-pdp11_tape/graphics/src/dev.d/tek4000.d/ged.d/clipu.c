static char SCCSID[]="@(#)clipu.c	1.1";
#include <stdio.h>
#include "ged.h"

clipu(x) /* clip x to universe */
long x;
{
	if( x>XYMAX ) return(XYMAX);
	else if( x<XYMIN ) return(XYMIN);
	else return(x);
}
