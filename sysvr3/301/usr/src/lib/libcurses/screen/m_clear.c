/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/m_clear.c	1.3"
# include	"curses.ext"

/*
 * Like m_erase but ensures that the screen will be cleared on next refresh.
 */
m_clear()
{
#ifdef DEBUG
	if (outf) fprintf(outf, "M_CLEAR\n");
#endif
	SP->doclear = 1;
	m_erase();
}
