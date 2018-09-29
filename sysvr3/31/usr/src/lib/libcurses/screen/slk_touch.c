/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_touch.c	1.3"
#include	"curses_inc.h"

/* Make the labels appeared changed. */

slk_touch()
{
    register	SLK_MAP	*slk;
    register	int	i;

    if (((slk = SP->slk) == NULL) || (slk->_changed == 2))
	return (ERR);

    for (i = 0; i < slk->_num; ++i)
	slk->_lch[i] = TRUE;
    slk->_changed = TRUE;

    return (OK);
}
