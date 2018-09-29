/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wattroff.c	1.11"
#include	"curses_inc.h"

wattroff(win,a)
WINDOW	*win;
chtype	a;
{
    /* if attribute contains color information, but this is not a color    */
    /* terminal, or that color information doesn't match the one stored	   */
    /* inside _attrs,  ignore that information.				   */

    if (((a & A_COLOR) && (SP->_pairs_tbl == NULL)) ||
        ((a & A_COLOR) != (win->_attrs & A_COLOR)))
	 a &= ~A_COLOR;

    if ((a & A_ATTRIBUTES) == A_NORMAL)
	 return (1);

    /* turn off the attributes		*/

    win->_attrs &= ~a & A_ATTRIBUTES;

    /* if background contains color information different from the one */
    /* we have just turned off, turn that color on.  (Reason: the      */
    /* color we have just turned off was set by wattron(), so the back-*/
    /* ground color was blocked.  However, now the background color can*/
    /* be seen.							       */

    if ((a & A_COLOR) && ((a & A_COLOR) != (win->_bkgd & A_COLOR)))
	 win->_attrs |= (win->_bkgd & A_COLOR);

    return (1);
}
