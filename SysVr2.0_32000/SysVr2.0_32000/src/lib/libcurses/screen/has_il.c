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
/*	@(#) has_il.c: 1.1 10/15/83	(1.19	3/17/83)	*/

/*
 * Queries: does the terminal have insert/delete line?
 */
has_il()
{
	return insert_line && delete_line || change_scroll_region;
}
