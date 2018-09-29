/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/searchlist.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lp.h"

/**
 ** searchlist() - SEARCH (char **) LIST FOR ITEM
 **/

int			searchlist (item, list)
	register char		*item;
	register char		**list;
{
	if (!list || !*list)
		return (0);

	else if (STREQU(item, NAME_ANY))
		return (1);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (*list) {
		if (STREQU(*list, item) || STREQU(*list, NAME_ANY))
			return (1);
		list++;
	}
	return (0);
}
