/*@(#)bext_h.c	1.1*/
#include "bit.h"

/* Extract a subfield of len bits from m starting with bit position i
 * and extending left for len bits. The right most bit is bit 0.
 */

short bext_h(m,i,len)
short *m, *i, *len;
{
	if ( (*i + *len > NBSI)  ||  (*i < 0)  ||  (*len < 0)  ||
	     (*i >= NBSI) || (*len > NBSI) ) return(*m);
	return((*m & F77mask[*i + *len]) >> *i);
}
