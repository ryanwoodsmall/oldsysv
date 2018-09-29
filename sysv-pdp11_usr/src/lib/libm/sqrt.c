/*	@(#)sqrt.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	sqrt returns the square root of its double-precision argument,
 *	using Newton's method.
 *	Returns EDOM error and value 0 if argument negative.
 *	Calls frexp and ldexp.
 */

#include <errno.h>
#include <math.h>
#define ITERATIONS	4

double
sqrt(x)
register double x;
{
	register double y;
	int iexp;
	register int i;
	struct exception exc;

	if (x <= 0) {
		if (x != 0) {
			exc.type = DOMAIN;
			exc.name = "sqrt";
			exc.arg1 = x;
			if (!matherr(&exc)) {
				write(2,"sqrt: DOMAIN error\n",19);
				exc.retval = 0.0;
				errno = EDOM;
				}
			return (exc.retval);
			}
		return (0);
	}
	y = frexp(x, &iexp);
	if (iexp % 2) {
		iexp--;
		y += y;
	}
	y = ldexp(y + 1, iexp/2 - 1);
	for (i = ITERATIONS; --i >= 0; )
		y = 0.5 * (y + x / y);
	return (y);
}
