/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:shift_h.c	1.3"
#include "bit.h"

/* Logically shift m k-places. If k > NBLI, m = undefined.
 * k > 0 => left shift.
 * k < 0 => right shift.
 * k = 0 => no shift.
 */

short shft_h(m,k)
short *m, *k;
{
	short l, left;

	left = (*k > 0);
	l = left ? *k : -*k;

	if (l > 0 && l < NBSI)
		if (left)
			return(*m << l);
		else
			return( (*m >> l) & F77mask[NBSI - l] );
	else if (l == 0)
		return(*m);
	else return(0);
}
