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
#include "curses.ext"
/*	@(#) erasechar.c: 1.1 10/15/83	(1.19	3/17/83)	*/

char
erasechar()
{
#ifdef USG
	return cur_term->Ottyb.c_cc[VERASE];
#else
	return cur_term->Ottyb.sg_erase;
#endif
}
