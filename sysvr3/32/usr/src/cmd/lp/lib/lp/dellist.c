/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/dellist.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lp.h"

extern char		*realloc();

extern void		free();

/**
 ** dellist() - REMOVE ITEM FROM (char **) LIST
 **/

int			dellist (plist, item)
	register char		***plist,
				*item;
{
	register char		**pl;

	register int		n;

	if (*plist) {

		n = lenlist(*plist);

		for (pl = *plist; *pl; pl++)
			if (STREQU(*pl, item))
				break;

		if (*pl) {
			free (*pl);
			for (; *pl; pl++)
				*pl = *(pl+1);
			if (--n == 0) {
				free ((char *)*plist);
				*plist = 0;
			} else {
				*plist = (char **)realloc(
					(char *)*plist,
					(n + 1) * sizeof(char *)
				);
				if (!*plist)
					return (-1);
/*				(*plist)[n] = 0; /* done in "for" loop */
			}
		}

	}

	return (0);
}
