/*@(#)bset_h.c	1.1*/
#include "bit.h"

/* The result is equal to the value of n with the ith bit set to 1.
 * The right most bit is bit 0.
 */

short bset_h(n,i)
short *n, *i;
{
	if (*i < 0 || *i >= NBSI)
		return(0L);
	return(*n | (1L << *i));
}
