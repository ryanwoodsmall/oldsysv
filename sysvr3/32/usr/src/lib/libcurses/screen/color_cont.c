/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:color_cont.c	1.2"

#include "curses_inc.h"

color_content(color, r, g, b)
short  color;
short  *r, *g, *b;
{
    register _Color *ctp;

    if (color < 0 || color > COLORS || !can_change)
        return (ERR);

    ctp = SP->_color_tbl + color;
    *r = ctp->r;
    *g = ctp->g;
    *b = ctp->b;
    return (OK);
}
