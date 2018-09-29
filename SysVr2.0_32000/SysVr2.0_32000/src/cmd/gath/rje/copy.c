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
/*
 * Copy N bytes of data from SRC to DST
 */

static char Sccsid[] = "@(#)copy.c	1.1";

copy(src,dst,n)
register char *src,*dst;
register int n;
{
	do {
		*dst++ = *src++;
	} while(--n);
}
