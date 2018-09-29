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
/*	@(#)h_sign.c	1.2	*/
short h_sign(a,b)
short *a, *b;
{
short x;
x = (*a >= 0 ? *a : - *a);
return( *b >= 0 ? x : -x);
}
