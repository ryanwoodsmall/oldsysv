/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)fmod.c	1.7	*/
/*LINTLIBRARY*/
/*
 *	fmod(x, y) returns the remainder of x on division by y,
 *	with the same sign as x,
 *	except that if |y| << |x|, it returns 0.
 */

#include <math.h>

double
fmod(x, y)
register double x, y;
{
	double d; /* can't be in register because of modf() below */

	/*
	 * The next lines determine if |y| is negligible compared to |x|,
	 * without dividing, and without adding values of the same sign.
	 */
	d = _ABS(x);
	if (d - _ABS(y) == d)
		return (0.0);
#ifndef	pdp11	/* pdp11 "cc" can't handle cast of double to void */
	(void)
#endif
	modf(x/y, &d); /* now it's safe to divide without overflow */
	return (x - d * y);
}
