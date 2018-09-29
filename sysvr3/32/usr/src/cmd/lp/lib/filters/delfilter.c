/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/delfilter.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "string.h"

#include "lp.h"
#include "filters.h"

extern char		*realloc();

extern void		free();

/**
 ** delfilter() - DELETE A FILTER FROM FILTER TABLE
 **/

int			delfilter (name)
	char			*name;
{
	register _FILTER	*pf;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		trash_filters ();
		goto Done;
	}

	/*
	 * Don't need to check for ENOENT, because if it is set,
	 * well that's what we want to return anyway!
	 */
	if (!filters && get_and_load() == -1 /* && errno != ENOENT */ )
		return (-1);

	if (!(pf = search_filter(name))) {
		errno = ENOENT;
		return (-1);
	}

	freefilter (pf);
	for (; pf->name; pf++)
		*pf = *(pf+1);

	nfilters--;
	filters = (_FILTER *)realloc(
		(char *)filters, (nfilters + 1) * sizeof(_FILTER)
	);
	if (!filters) {
		errno = ENOMEM;
		return (-1);
	}

/*	filters[nfilters].name = 0;	/* last for loop above did this */

Done:	return (dumpfilters((char *)0));
}
