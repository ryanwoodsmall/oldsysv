/*	@(#)sinh.c	1.4	*/
/*LINTLIBRARY*/
/*
 *	sinh(arg) returns the hyperbolic sine of its floating-
 *	point argument.
 *
 *	The exponential function is called for arguments
 *	greater in magnitude than 0.5.
 *
 *	A series is used for arguments smaller in magnitude than 0.5.
 *	The coefficients are #2029 from Hart & Cheney. (20.36D)
 *
 *	cosh(arg) is computed from the exponential function for
 *	all arguments.
 */

#include <math.h>
#include <errno.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define LOGHUGE	((DMAXEXP + 1) / LOG2E)
#define	LOG2E	1.4426950408889634074

double
sinh(x)
register double x;
{
	register double y = _ABS(x);
	struct exception exc;

	if (y < 0.5) {
		static double p[] = {
			-0.2630563213397497062819489e+2,
			-0.2894211355989563807284660366e+4,
			-0.8991272022039509355398013511e+5,
			-0.6307673640497716991184787251e+6,
		}, q[] = {
			 1.0,
			-0.173678953558233699533450911e+3,
			 0.1521517378790019070696485176e+5,
			-0.6307673640497716991212077277e+6,
		};

		if (y < XMIN) /* for efficiency and to prevent underflow */
			return (x);
		y = x * x;
		return (x * _POLY3(y, p)/_POLY3(y, q));
	}
	/*
	 * A special test should be added here for those cases where exp(|x|)
	 * overflows while exp(|x|)/2 is still representable.
	 */
	if (y > LOGHUGE){
		exc.type = OVERFLOW;
		exc.name = "sinh";
		exc.arg1 = x;
		if (!matherr(&exc)){
			errno = ERANGE;
			exc.retval = HUGE;
			}
		return (exc.retval);
		}
	x = exp(x);
	return (0.5 * (x - 1.0/x));
}

double
cosh(x)
register double x;
{
	struct exception exc;
	
        if (x > LOGHUGE){
                exc.type = OVERFLOW;
                exc.name = "cosh";
                exc.arg1 = x;
                if (!matherr(&exc)){
                        errno = ERANGE;
                        exc.retval = HUGE;
                        }
                return (exc.retval);
                }
	x = exp(x);
	return (0.5 * (x + 1.0/x));
}
