/*	@(#)gamma.c	1.10	*/
/*LINTLIBRARY*/
/*
 *	gamma(x) computes the log of the absolute
 *	value of the gamma function.
 *	The sign of the gamma function is returned in the
 *	external integer signgam.
 *
 *	The coefficients for expansion around zero
 *	are #5243 from Hart & Cheney; for expansion
 *	around infinity they are #5404.
 *
 *	Calls log and sin.
 */

#include <errno.h>
#include <math.h>
#define	LOG2E	1.4426950408889634074
#define LOGHUGE ((DMAXEXP + 1) / LOG2E)
#define XMAX	(3.0 * (1L << (DSIGNIF/2)))
#define GOOBIE	0.9189385332046727417803297

extern double neg_gamma(), pos_gamma();

int signgam;

double
gamma(x)
register double x;
{
	signgam = 1;
	return (x > 0 ? pos_gamma(x) : neg_gamma(-x));
}

static double
neg_gamma(x)
register double x;
{
	static double pi = 3.1415926535897932384626434;
	double temp;
	struct exception exc;

	exc.name = "gamma";
	exc.arg1 = x;
	if (modf(x, &temp) == 0) { /* x is integral */
		exc.type = DOMAIN;
		if (!matherr(&exc)){
			write(2,"gamma: DOMAIN error\n",20);
			exc.retval = HUGE;
			errno = EDOM;
			}
		return (exc.retval);
		}
	if (x >= XMAX) {
		exc.type = OVERFLOW;
		exc.retval = HUGE;
		if (!matherr(exc)) errno = ERANGE;
		return (exc.retval);
		}
	if ((temp = sin(pi * x)) < 0.0)
		temp = -temp;
	else
		signgam = -1;
	return (-(log(x * temp/pi) + pos_gamma(x)));
}

static double
pos_gamma(x)
register double x;
{
	static double p2[] = {
		-0.67449507245925289918e1,
		-0.50108693752970953015e2,
		-0.43933044406002567613e3,
		-0.20085274013072791214e4,
		-0.87627102978521489560e4,
		-0.20886861789269887364e5,
		-0.42353689509744089647e5,
	}, q2[] = {
		 1.0,
		-0.23081551524580124562e2,
		 0.18949823415702801641e3,
		-0.49902852662143904834e3,
		-0.15286072737795220248e4,
		 0.99403074150827709015e4,
		-0.29803853309256649932e4,
		-0.42353689509744090010e5,
	};
	register double factor;

	if (x > 8.0) { /* assymptotic approximation */
		static double p[] = {
			-0.1633436431e-2,
			 0.83645878922e-3,
			-0.5951896861197e-3,
			 0.793650576493454e-3,
			-0.277777777735865004e-2,
			 0.83333333333333101837e-1,
		};
		register double y = 0.0;
	
		if (x < XMAX) {
			y = 1.0 / (x * x);
			y = _POLY5(y, p)/x;
		}
		else if (x >= HUGE/LOGHUGE) {
			return (HUGE);
			}
		return ((x - 0.5) * log(x) - x + GOOBIE + y);
	}
	if (x < 1.0)
		factor = 1.0 / (x * (1.0 + x));
	else if (x < 2.0) {
		factor = 1.0 / x;
		x -= 1.0;
	} else {
		for (factor = 1.0; x >= 3.0; factor *= x)
			x -= 1.0;
		x -= 2.0;
	}
	return (log(factor * _POLY6(x, p2)/_POLY7(x, q2)));
}
