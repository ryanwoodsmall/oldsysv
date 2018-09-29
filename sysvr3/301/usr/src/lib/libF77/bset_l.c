/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:bset_l.c	1.2"
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
