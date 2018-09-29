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
/*	@(#)fread.c	3.11	*/
/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include "stdiom.h"

#define MIN(x, y)	(x < y ? x : y)

extern int _filbuf();
extern _bufsync();
extern char *memcpy();

int
fread(ptr, size, count, iop)
char *ptr;
int size, count;
register FILE *iop;
{
	register unsigned int nleft;
	register int n;

	if (size <= 0 || count <= 0) return 0;
	nleft = count * size;

	/* Put characters in the buffer */
	/* note that the meaning of n when just starting this loop is
	   irrelevant.  It is defined in the loop */
	for ( ; ; ) {
		if (iop->_cnt <= 0) { /* empty buffer */
			if (_filbuf(iop) == EOF)
				return (count - (nleft + size - 1)/size);
			iop->_ptr--;
			iop->_cnt++;
		}
		n = MIN(nleft, iop->_cnt);
		ptr = memcpy(ptr, (char *) iop->_ptr, n) + n;
		iop->_cnt -= n;
		iop->_ptr += n;
		_BUFSYNC(iop);
		if ((nleft -= n) <= 0)
			return (count);
	}
}
