/*@(#)btest_l.c	1.1*/
#include "bit.h"

/* The ith bit of argument n is tested. If it is 1 return .TRUE.,
 * otherwise return .FALSE. The right most bit is bit 0.
 */

long btest_l(n,i)
long *n, *i;
{
	if (*i < 0 || *i >= NBLI)
		return(0L);
	return((*n & F77zmask[*i]) >> *i);
}
