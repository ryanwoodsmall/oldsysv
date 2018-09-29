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
/*	@(#)tell.c	1.1	*/

# include <stand.h>

off_t
tell(fildes)
{
	extern off_t lseek();

	return (lseek(fildes, (off_t) 0, 1));
}
