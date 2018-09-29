/*@(#)mvbits.c	1.1*/
#include "bit.h"

/* Move len bits from position i through i+len-1 of argument m to
 *		      position j through j+len-1 of argument n.
 * The right most bit is bit 0.
 */

long mvbits_(m,i,len,n,j)
long *m, *i, *len, *n, *j;
{
	unsigned long b;

	if ( (*i + *len > NBLI) || (*j + *len > NBLI) ||
	     (*len <= 0) || (*i < 0) || (*j < 0) ) return;

	*n &= ~(F77mask[*j + *len] ^ F77mask[*j]);	/* clear dest field */
	b = (*m >> *i) & F77mask[*len];		/* extract bits from src */
	*n |= (b << *j);			/* position bits and insert */
}
