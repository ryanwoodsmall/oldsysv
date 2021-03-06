/*	@(#)exp.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	exp returns the exponential function of its double-precision argument.
 *	Returns ERANGE error and value 0 if argument too small,
 *	   value HUGE if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls ldexp.
 */

#include <math.h>
#include <errno.h>
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define LOGTINY	((DMINEXP - 1) / LOG2E)
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define LOG2E	1.4426950408889634074
#define HALF	q[3] /* 0.5 */

double
exp(arg)
register double arg;
{
	struct exception exc;
	static double p[] = {
		0.31555192765684646356e-4,
	        0.75753180159422776666e-2,
	        0.25000000000000000000e0,
	}, q[] = {
		0.75104028399870046114e-6,
	        0.63121894374398503557e-3,
	        0.56817302698551221787e-1,
	        0.50000000000000000000e0,
	};
	register double x, y;
	register int n;

	if (arg < LOGTINY) {
		exc.type = UNDERFLOW;
		exc.name = "exp";
		exc.arg1 = arg;
		exc.retval = 0.0;
		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
	if (arg > LOGHUGE) {
		exc.type = OVERFLOW;
		exc.name = "exp";
		exc.arg1 = arg;
		exc.retval = HUGE;
		if (!matherr(&exc)) 
			errno = ERANGE;
		return (exc.retval);
	}
	if ((x = _ABS(arg)) < XMIN)
		return (1);
	n = (int)(x * LOG2E + HALF);
	y = (double)n;
	_REDUCE(int, x, y, 0.693359375, -2.1219444005469058277e-4);
	if (arg < 0) {
		x = -x;
		n = -n;
	}
	y = x * x;
	x *= _POLY2(y, p);
	return (ldexp(HALF + x/(_POLY3(y, q) - x), n + 1));
}
