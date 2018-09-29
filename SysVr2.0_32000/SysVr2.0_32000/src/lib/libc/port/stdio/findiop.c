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
/*	@(#)findiop.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

extern FILE *_lastbuf;

FILE *
_findiop()
{
	register FILE *iop;

	for(iop = _iob; iop->_flag & (_IOREAD | _IOWRT | _IORW); iop++)
		if(iop >= _lastbuf)
			return(NULL);
	return(iop);
}
