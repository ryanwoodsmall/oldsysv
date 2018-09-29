/*	@(#)hypot.c	1.6	*/
/*LINTLIBRARY*/

/*
 *	sqrt(a^2 + b^2)
 *	(but carefully)
 */

#include <errno.h>
#include <math.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define SQ_TWO	1.41421356237309504880

double
hypot(a, b)
register double a, b;
{
	struct exception exc;

	if ((exc.arg1 = a) < 0)
		a = -a;
	if ((exc.arg2 = b) < 0)
		b = -b;
	if (a > b) {				/* make sure a <= b */
		double t = a;
		a = b;
		b = t;
	}
	if ((a /= b) < XMIN)			/* a <= 1 */
		return (b);			/* a << 1 */
	a = a * a;
	/* use first term of Taylor series for very small angles */
	a = (a < XMIN) ? 1.0 + 0.5 * a : sqrt(1.0 + a);	/* a <= sqrt(2) */
	if (b < MAXDOUBLE/SQ_TWO)		/* result can't overflow */
		return (a * b);
	if ((a *= 0.5 * b) < MAXDOUBLE/2)	/* result won't overflow */
		return (a + a);
	exc.type = OVERFLOW;
	exc.name = "hypot";
	exc.retval = HUGE;
	if (!matherr(&exc))
		errno = ERANGE;
	return (exc.retval);
}
