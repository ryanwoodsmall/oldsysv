/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)c_sin.c	1.2	*/
#include "complex"

c_sin(r, z)
complex *r, *z;
{
double sin(), cos(), sinh(), cosh();

r->real = sin(z->real) * cosh(z->imag);
r->imag = cos(z->real) * sinh(z->imag);
}
