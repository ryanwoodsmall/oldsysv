/*	@(#)shift_l.c	1.2	*/
#include "bit.h"

/* Logically shift m k-places. If k > NBLI, m = undefined.
 * k > 0 => left shift.
 * k < 0 => right shift.
 * k = 0 => no shift.
 */

long shft_l(m,k)
long *m, *k;
{
	long l;
	short left;

	left = (*k > 0);
	l = left ? *k : -*k;

	if (l > 0 && l < NBLI)
		if (left)
			return(*m << l);
		else
			return( (*m >> l) & F77mask[NBLI - l] );
	else if (l == 0)
		return(*m);
	else return(0L);
}
