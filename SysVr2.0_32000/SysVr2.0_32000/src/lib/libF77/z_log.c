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
/*	@(#)z_log.c	1.2	*/
#include "complex"

z_log(r, z)
dcomplex *r, *z;
{
double log(), cabs(), atan2();

r->dimag = atan2(z->dimag, z->dreal);
r->dreal = log( cabs( z->dreal, z->dimag ) );
}
