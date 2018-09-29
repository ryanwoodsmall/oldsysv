/*	@(#)sin.c	1.6	*/
/*LINTLIBRARY*/
/*
 *	C program for double-precision sin/cos.
 *	Returns EDOM error and value 0 if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <errno.h>
#define XMIN	(1.0/(1L << (DSIGNIF/2)))
#define XMAX	(3.0 * (1L << (DSIGNIF/2)))
#define ONE_OVER_PI	0.31830988618379067154
#define PI_OVER_TWO	1.57079632679489661923

double
sin(x)
double x;
{
	extern double sin_cos();

	return (sin_cos(x, 0));
}

static double
sin_cos(x, cosflag)
double x;
int cosflag;
{
	double y;
	struct exception exc;
	register int neg;
	register long n;
	
	exc.arg1 = x;
	if (neg = (x < 0))
		x = -x;
	y = x;
	if (cosflag) {
		exc.name = "cos";
		neg = 0;
		y += PI_OVER_TWO;
		}
	else exc.name = "sin";
	if (y > XMAX){
		exc.type = TLOSS;
		exc.retval = 0.0;
		if (!matherr(exc)) {errno = ERANGE;
			write(2,exc.name,3);
			write(2,": TLOSS error\n",14);
			}
		return(exc.retval);
		}
	n = (long)(y * ONE_OVER_PI + 0.5);
	y = (double)n;
	if (cosflag)
		y -= 0.5;
	_REDUCE(long, x, y, 3.1416015625, -8.908910206761537356617e-6);
	if (_ABS(x) > XMIN) {
		/* skip for efficiency and to prevent underflow */
		static double p[] = {
			 0.27204790957888846175e-14,
			-0.76429178068910467734e-12,
			 0.16058936490371589114e-9,
			-0.25052106798274584544e-7,
			 0.27557319210152756119e-5,
			-0.19841269841201840457e-3,
			 0.83333333333331650314e-2,
			-0.16666666666666665052e0,
		};

		y = x * x;
		x += x * y * _POLY7(y, p);
	}
	return (neg ^ (int)(n % 2) ? -x : x);
}

double
cos(x)
double x;
{
	return (sin_cos(x, 1));
}
