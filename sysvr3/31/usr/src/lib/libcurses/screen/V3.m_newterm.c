/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.m_newterm.c	1.1"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
SCREEN	*
m_newterm(type, outfptr, infptr)
char	*type;
FILE	*outfptr, *infptr;
{
    return (newterm(type, outfptr, infptr));
}
#endif	/* _VR3_COMPAT_CODE */
