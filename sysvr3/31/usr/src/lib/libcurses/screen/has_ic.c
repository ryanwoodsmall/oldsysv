/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/has_ic.c	1.3"

#include "curses_inc.h"

/* Query: Does it have insert/delete char? */

has_ic()
{
    return ((insert_character || enter_insert_mode || parm_ich) &&
		(delete_character || parm_dch));
}
