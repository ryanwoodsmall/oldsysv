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
/*	@(#)d_mod.c	1.2	*/
double d_mod(x,y)
double *x, *y;
{
double floor(), quotient;
if( (quotient = *x / *y) >= 0)
	quotient = floor(quotient);
else
	quotient = -floor(-quotient);
return(*x - (*y) * quotient );
}
