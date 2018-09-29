/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/class/freeclass.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lp.h"
#include "class.h"

/**
 ** freeclass() - FREE SPACE USED BY CLASS STRUCTURE
 **/

void			freeclass (clsbufp)
	CLASS			*clsbufp;
{
	if (!clsbufp)
		return;
	if (clsbufp->name)
		free (clsbufp->name);
	freelist (clsbufp->members);
	return;
}
