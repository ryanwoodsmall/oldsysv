/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.vidattr.c	1.2"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	vidattr

vidattr(a)
_ochtype	a;
{
    vidupdate(_FROM_OCHTYPE(a), cur_term->sgr_mode, _outchar);
    return (OK);
}
#endif	/* _VR3_COMPAT_CODE */
