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
/*	@(#) fixterm.c: 1.1 10/15/83	(1.36	3/18/83)	*/
#include "curses.ext"
#include "../local/uparm.h"

extern	struct term *cur_term;

fixterm()
{
	reset_prog_mode();
}
