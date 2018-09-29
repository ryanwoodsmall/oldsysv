/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V2.makenew.c	1.2"

#include "curses.ext"

#ifdef _VR2_COMPAT_CODE
extern WINDOW *_makenew();

WINDOW *
makenew(num_lines, num_cols, begy, begx)
int	num_lines, num_cols, begy, begx;
{
	return _makenew(num_lines, num_cols, begy, begx);
}
#endif
