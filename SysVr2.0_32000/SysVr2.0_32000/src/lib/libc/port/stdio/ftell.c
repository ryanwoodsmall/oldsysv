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
/*	@(#)ftell.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Return file offset.
 * Coordinates with buffering.
 */
#include <stdio.h>

extern long lseek();

long
ftell(iop)
FILE	*iop;
{
	long	tres;
	register int adjust;

	if(iop->_cnt < 0)
		iop->_cnt = 0;
	if(iop->_flag & _IOREAD)
		adjust = - iop->_cnt;
	else if(iop->_flag & (_IOWRT | _IORW)) {
		adjust = 0;
		if(iop->_flag & _IOWRT && iop->_base &&
					(iop->_flag & _IONBF) == 0)
			adjust = iop->_ptr - iop->_base;
	} else
		return(-1);
	tres = lseek(fileno(iop), 0L, 1);
	if(tres >= 0)
		tres += (long)adjust;
	return(tres);
}
