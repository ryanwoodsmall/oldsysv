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
/*	@(#)ungetc.c	2.1	*/
/*	3.0 SID #	1.3	*/
/*LINTLIBRARY*/
#include <stdio.h>

int
ungetc(c, iop)
int	c;
register FILE *iop;
{
	if(c == EOF)
		return(-1);
	if((iop->_flag & _IOREAD) == 0 || iop->_ptr <= iop->_base)
		if(iop->_ptr == iop->_base && iop->_cnt == 0)
			++iop->_ptr;
		else
			return(-1);
	*--iop->_ptr = c;
	++iop->_cnt;
	return(c);
}
