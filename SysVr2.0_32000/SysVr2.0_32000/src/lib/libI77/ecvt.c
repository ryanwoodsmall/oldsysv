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
/*	@(#)ecvt.c	1.2	*/
/*LINTLIBRARY*/
/*
 *	ecvt converts to decimal
 *	the number of digits is specified by ndigit
 *	decpt is set to the position of the decimal point
 *	sign is set to 0 for positive, 1 for negative
 *
 */
#include <nan.h>
#include <values.h>

#define	IEEE	( u3b || ns32000 )

#define	NMAX	((DSIGNIF * 3 + 19)/10) /* restrict max precision */
#if u3b || u3b5 || ns32000
#define NDIG	330
#else
#define	NDIG	84
#endif

/* The following macro converts a binary exponent to a decimal one. */
#define E2to10(x)	(((x)*301029L + 500000000L)/1000000L - 500)

/* LONGBITS is the number of bits in a long NOT including the sign bit */
#define	LONGBITS	(BITSPERBYTE * sizeof(long) - 1)
#define	LONGDIGITS	E2to10(LONGBITS)
#define	TEN_TO_(y)	((ten_pow-1)[y])
#define	_1e16_TO_(y)	((_1e16_pow-1)[y])

extern char *cvt();
extern char *ltostr();
extern double frexp();

char *
ecvt(value, ndigit, decpt, sign)
double	value;
int	ndigit, *decpt, *sign;
{
	return (cvt(value, ndigit, decpt, sign, 0));
}

char *
fcvt(value, ndigit, decpt, sign)
double	value;
int	ndigit, *decpt, *sign;
{
	return (cvt(value, ndigit, decpt, sign, 1));
}

double ten_pow[] = {1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9};
static char buf[NDIG];
static char *
cvt(value, ndigit, decpt, sign, f_flag)
double value;
int	ndigit, *sign, f_flag;
int	*decpt;
{
	register double tvalue;
	int binexp, tdecpt, shift;
	long longvalue;
	char *begin_ptr, *end_ptr;

	/* The maximum exponent in the ten_pow table should equal LONGDIGITS, */
	/* but it is hard to make this automatic.  Check this when porting!   */
	/* The size of the _1e16_pow table is also machine dependent.	      */
	static double _1e16_pow[] = {1e16, 1e32,
#if IEEE
					1e48, 1e64,
#endif
					};

	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
	if (*sign = (value < 0.0))
		value = -value;

	if ((tvalue = value) == 0.0) {
		tdecpt = 0;
		*(begin_ptr = &buf[0]) = '0';
		end_ptr = &buf[1];
	}
	else {

		/*
		   Non-zero value.  Estimate "tdecpt", the number of digits
		   before the decimal point, erring on the low side.  This
		   value may be negative.
		*/

#ifndef pdp11
		(void) frexp(tvalue, &binexp);
#else
		frexp(tvalue, &binexp);
#endif
		tdecpt = E2to10(binexp - 1) + 1;

		if (f_flag) {	/* Fortran f format conversion */
			register int totdigits;
			if ((totdigits = ndigit + tdecpt) < 0)
				totdigits = -1;
			ndigit = totdigits;
		}

		/*
		   Decide how many digits to try to convert in the first
		   longword ("length_wanted").  This will be the number
		   of digits desired plus a rounding digit (i.e. ndigit+1)
		   unless that is too much to fit in a long (allowing one
		   bit margin since "tvalue" may be up to one bit larger
		   than revealed by "tdecpt").
		   Next compute "shift", the number of decimal places by
		   which tvalue must be shifted left so that "length_wanted"
		   digits (and possibly one more) will be in front of the
		   decimal point.
		   Then carry out the shift.
		*/

		{   register int length_wanted;
		    {	register int tshift, t;

			if ((length_wanted = ndigit + 1) > E2to10(LONGBITS - 1))
				length_wanted = E2to10(LONGBITS - 1);
			shift = tshift = length_wanted - tdecpt;
#if IEEE
			while (tshift > 73) {
				tvalue *= 1e146;
				tshift -= 146;
			}
			while (tshift < -73) {
				tvalue /= 1e146;
				tshift += 146;
			}
#endif
			if (tshift > 9) {
				tvalue *= _1e16_TO_((t = tshift + 6) >> 4);
				tshift -= t & 0xff0;
			}
			else if (tshift < -9) {
				tvalue /= _1e16_TO_((t = -tshift + 6) >> 4);
				tshift += t & 0xff0;
			}
			if (tshift > 0)
				tvalue *= TEN_TO_(tshift);
			else if (tshift < 0)
				tvalue /= TEN_TO_(-tshift);
		    }

		/*
		   Convert the part of tvalue which is now before the
		   decimal point, and make the necessary adjustments
		   if we underestimated the number of digits before the
		   decimal point.
		   Then compute the number of additional digits needed--
		   to be obtained by the conversion of another long.
		*/

		    {	register int digits_obtained;
			register int adjustment;

			begin_ptr = ltostr((longvalue = (long) tvalue),
					(end_ptr = &buf[4] + length_wanted));

			digits_obtained = end_ptr - begin_ptr;
			if ((adjustment = digits_obtained
							- length_wanted) > 0) {
				tdecpt += adjustment;
				if (f_flag)
					ndigit += adjustment;
			}

			if (ndigit > NMAX)
				ndigit = NMAX;

			if ((length_wanted = ndigit + 1 - digits_obtained)
						> LONGDIGITS)
				length_wanted = LONGDIGITS;

			end_ptr += length_wanted; /* length_wanted may be < 0 */
		    }

		/*
		   To preserve accuracy, some of the digits to be placed in the
		   second long may be obtained, if necessary, by subtracting
		   from the original value that part which has already been
		   converted (the converted portion of the the shifted value is
		   "unshifted" to its original position and subtracted, giving
		   the effect of producing "unshift" new digits, to which
		   "new_shift" digits are appended by an additional shift);
		   otherwise the fractional part of tvalue will be shifted left.
		*/

		    if (length_wanted > 0) {

		    	{   register int unshift, new_shift;

			    if ((unshift = -shift) > 0 && 
				   (new_shift = length_wanted - unshift) >= 0) {

				tvalue = value - ((double) longvalue)
							* TEN_TO_(unshift);
				if (new_shift > 0)
					tvalue *= TEN_TO_(new_shift);
			    }
			    else
				tvalue = (tvalue - (double) longvalue)
						    * TEN_TO_(length_wanted);
			}

			/*
			   Convert the second long, and fill out with leading
			   zeroes until "length_wanted" digits are obtained.
			*/

			{   register char *p, *tend_ptr = end_ptr;

			    p = ltostr((long) tvalue, tend_ptr);

			    while (tend_ptr - p < length_wanted)
					*--p = '0';
			}
		    }
		}
	}

	/*
	   Round result.  If the rightmost converted digit is 5 or more,
	   1 is added to the next digit to the left; the carry propagates
	   as long as 9's are encountered; if the number is all 9's, it
	   turns to 100000....., and the position of the decimal point is
	   adjusted.   Finally, trailing zeroes are appended as required.
	*/

	{	register char *p, *tend_ptr = end_ptr - 1;

#if !IEEE
		if (ndigit == NMAX && tend_ptr - begin_ptr < NMAX)
		    tend_ptr++;
		else
#endif
		    if (*(p = tend_ptr) >= '5')
			do {
				*p = '0';
				if (p == begin_ptr) { /* all 9's */
					*--begin_ptr = '1';
					++tdecpt;
					if (f_flag)
						++ndigit;
					break;
				}
			} while (++*--p > '9');

		if ((p = begin_ptr + ndigit) >= &buf[NDIG-1] )
			p = &buf[NDIG-1];  /* don't allow buffer overflow */
		while (tend_ptr < p)
			*tend_ptr++ = '0'; /* append zeroes */
		*p = '\0';
	}

	*decpt = tdecpt;
	return(begin_ptr);
}
