/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libF77:c_exp.c	1.3"
#include "complex"

c_exp(r, z)
complex *r, *z;
{
double expx;
double exp(), cos(), sin();

expx = exp(z->real);
r->real = expx * cos(z->imag);
r->imag = expx * sin(z->imag);
}
