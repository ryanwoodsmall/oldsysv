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
/*	@(#)d_sign.c	1.3	*/
double d_sign(a,b)
double *a, *b;
{
double x;
	x = *a;
	if (x <= 0)
		x = -x;
	if (*b < 0)
		x = -x;
	return (x);
/*x = (*a >= 0 ? *a : - *a);
 *return( *b >= 0 ? x : -x);
*/
}
