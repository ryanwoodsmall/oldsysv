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
/*	@(#)space.c	1.1	*/
#include <stdio.h>
space(x0,y0,x1,y1){
	putc('s',stdout);
	putsi(x0);
	putsi(y0);
	putsi(x1);
	putsi(y1);
}
