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
/*	@(#)c_log.c	1.2	*/
#include "complex"

c_log(r, z)
complex *r, *z;
{
double log(), cabs(), atan2();

r->imag = atan2(z->imag, z->real);
r->real = log( cabs(z->real, z->imag) );
}
