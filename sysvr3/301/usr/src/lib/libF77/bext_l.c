/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:bext_l.c	1.2"
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
