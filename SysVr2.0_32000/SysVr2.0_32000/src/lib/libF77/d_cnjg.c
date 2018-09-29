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
/*	@(#)d_cnjg.c	1.2	*/
#include "complex"

d_cnjg(r, z)
dcomplex *r, *z;
{
r->dreal = z->dreal;
r->dimag = - z->dimag;
}
