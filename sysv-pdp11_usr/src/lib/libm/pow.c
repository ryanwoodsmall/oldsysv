/*	@(#)pow.c	1.8	*/
/*LINTLIBRARY*/
/*
 *	computes x^y.
 *	uses log and exp
 */

#include <errno.h>
#include <math.h>
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define LOGTINY	((DMINEXP - 1) / LOG2E)
#define LOG2E	1.4426950408889634074

double
pow(x, y)
register double x, y;
{
	register long ly;
	register double z;
	struct exception exc;

	exc.name = "pow";
	exc.arg1 = x;
	exc.arg2 = y;
	if (x > 0) {
		if ((z = y * log(x)) > LOGHUGE) {
			exc.type = OVERFLOW;
			exc.retval = HUGE;
			if (!matherr(&exc)) errno = ERANGE;
			return (exc.retval);
			}
		if (z < LOGTINY) {
			exc.type = UNDERFLOW;
			exc.retval = 0;
			if (!matherr(&exc)) errno = ERANGE;
			return (exc.retval);
			}
		return (exp(z));
	}
	if (x == 0) {
		if (y <= 0)
			goto domain;
		return (0);
	}
	ly = (long)y;
	if ((double)ly != y) {
domain:
		exc.type = DOMAIN;
		exc.retval = 0.0;
		if (!matherr(&exc)){
			write(2,"pow: DOMAIN error\n",18);
			errno = EDOM;
			}
		return (exc.retval);
	}
	x = exp(y * log(-x));
	return (ly % 2 ? -x : x);
}
