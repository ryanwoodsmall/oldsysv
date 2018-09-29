/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/trash.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lp.h"
#include "filters.h"

/**
 ** trash_filters() - FREE ALL SPACE ALLOCATED FOR FILTER TABLE
 **/

void			trash_filters ()
{
	register _FILTER	*pf;

	if (filters) {
		for (pf = filters; pf->name; pf++)
			freefilter (pf);
		free ((char *)filters);
		nfilters = 0;
		filters = 0;
	}
	return;
}
