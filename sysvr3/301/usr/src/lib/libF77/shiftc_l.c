/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:shiftc_l.c	1.3"
#include "bit.h"

/* Circular shift - The right most ic bits are shifted circularly k-places.
 * k > 0 => left shift.
 * k < 0 => right shift.
 * k = 0 => no shift.
 *	left shift			 right shift
 *	[  b1   |   k2   |   d   ]       [  b1   |   d   |   k2   ]
 */

long shftc_l(m,k,ic)
long *m, *k, *ic;
{
	long b1, b2, b3, k2, d;
	short left;

	left = (*k > 0);
	k2 = left ? *k : -*k;
	if (k2 == 0 || k2 > NBLI || k2 > *ic || *ic < 1 || *ic > NBLI)
		return(*m);
	d = (*ic - k2);
	b1 = *m & ~F77mask[*ic];		/* mask off left most bits */

	if (left) {
		b2 = (*m & F77mask[d]) << k2;	/* get k2 bits */
		b3 = (*m & F77mask[*ic]) >> d;	/* get d bits  */
		}
	else {
		b2 = (*m & F77mask[k2]) << d;	/* get k2 bits */
		b3 = (*m & F77mask[*ic]) >> k2;	/* get d bits  */
		}
	return( b1 | b2 | b3 );
}
