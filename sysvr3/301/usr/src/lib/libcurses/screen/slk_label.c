/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/slk_label.c	1.1"
/*
 * This routine returns the character string currently plastered onto a
 * soft-label. Initial and final blanks are not included.
 */

#include "curses.ext"

char *slk_label(slknum)
register int slknum;
{
    register struct slkdata *SLK = SP->slk;
    if (slknum < 1 || slknum > 8 || !SLK)
        return NULL;
    return SLK->nblabel[slknum - 1];
}
