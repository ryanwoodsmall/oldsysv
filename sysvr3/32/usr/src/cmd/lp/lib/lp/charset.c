/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/charset.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lp.h"

/**
 ** search_cslist() - SEARCH CHARACTER SET ALIASES FOR CHARACTER SET
 **/

char			*search_cslist (item, list)
	register char		*item;
	register char		**list;
{
	register char		*alias;


	if (!list || !*list)
		return (0);

	else if (STREQU(item, NAME_ANY))
		return (item);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (*list) {
		alias = strchr(*list, '=');
		if (alias && STREQU(alias+1, item))
			return (*list);
		list++;
	}
	return (0);
}
