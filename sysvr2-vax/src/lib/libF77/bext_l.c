/*@(#)bext_l.c	1.1*/
#include "bit.h"

/* Extract a subfield of len bits from m starting with bit position i
 * and extending left for len bits. The right most bit is bit 0.
 */

long bext_l(m,i,len)
long *m, *i, *len;
{
	if ( (*i + *len > NBLI)  ||  (*i < 0)  ||  (*len < 0)  ||
	     (*i >= NBLI) || (*len > NBLI) ) return(*m);
	return((*m & F77mask[*i + *len]) >> *i);
}
