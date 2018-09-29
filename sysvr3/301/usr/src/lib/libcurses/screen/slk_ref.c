/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_ref.c	1.1"
/*
 * Update the soft-labels.
 */

#include "curses.ext"

slk_refresh()
{
    slk_noutrefresh();
    return doupdate();
}
