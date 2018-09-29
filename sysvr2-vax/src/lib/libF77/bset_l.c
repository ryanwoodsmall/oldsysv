/*@(#)bset_l.c	1.1*/
#include "bit.h"

/* The result is equal to the value of n with the ith bit set to 1.
 * The right most bit is bit 0.
 */

long bset_l(n,i)
long *n, *i;
{
	if (*i < 0 || *i >= NBLI)
		return(0L);
	return(*n | (1L << *i));
}
