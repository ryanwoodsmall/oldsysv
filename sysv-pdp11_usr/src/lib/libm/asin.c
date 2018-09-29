/*	@(#)asin.c	1.7	*/
/*LINTLIBRARY*/
/*
 *	asin(arg) and acos(arg) return the arcsin, arccos,
 *	respectively of their arguments.
 *
 *	Arctan is called after appropriate range reduction.
 *	This leads to very high errors for argument values near 1.0.
 */

#include <errno.h>
#include <math.h>
#define PI_OVER_2	1.57079632679489661923

double
asin(x)
register double x;
{
	struct exception exc;
	if (x >= -1 && x <= 1)
		return (atan2(x, sqrt(1.0 - x * x)));
	exc.type=DOMAIN;
	exc.name="asin";
	exc.arg1=x;
	if (!matherr(&exc)){
		write(2,"asin: DOMAIN error\n",19);
		errno = EDOM;
		exc.retval=0.0;
		}
	return (exc.retval);
}

double
acos(x)
register double x;
{
	struct exception exc;
	if (x >= -1 && x <= 1)
		return (PI_OVER_2 - atan2(x, sqrt(1.0 - x * x)));
	exc.type=DOMAIN;
	exc.name="acos";
	exc.arg1=x;
	if (!matherr(&exc)){
		write(2,"acos: DOMAIN error\n",19);
		errno = EDOM;
		exc.retval=0.0;
		}
	return (exc.retval);
}
