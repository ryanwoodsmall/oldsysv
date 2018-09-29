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
/*	@(#) _forcehl.c: 1.1 10/15/83	(1.21	2/11/83)	*/

#include "curses.ext"

/*
 * Output the string to get us in the right highlight mode,
 * no matter what mode we are currently in.
 */
_forcehl()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_forcehl().\n");
#endif
	SP->phys_gr = -1;
	_sethl();
}
