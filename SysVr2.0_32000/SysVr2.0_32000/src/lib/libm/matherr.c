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
/*	@(#)matherr.c	1.2	*/
/*LINTLIBRARY*/

#include <math.h>

/*ARGSUSED*/
int
matherr(x)
struct exception *x;
{
	return (0);
}
