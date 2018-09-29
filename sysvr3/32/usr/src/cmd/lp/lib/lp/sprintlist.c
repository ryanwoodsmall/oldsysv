/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/sprintlist.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"

#include "lp.h"

extern char		*malloc();

/**
 ** sprintlist() - FLATTEN (char **) LIST INTO (char *) LIST
 **/

char			*sprintlist (list)
	char			**list;
{
	register char		**plist,
				*p,
				*q;

	char			*ret;

	int			len	= 0;


	if (!list || !*list)
		return (0);

	for (plist = list; *plist; plist++)
		len += strlen(*plist) + 1;

	if (!(ret = malloc(len))) {
		errno = ENOMEM;
		return (0);
	}

	q = ret;
	for (plist = list; *plist; plist++) {
		p = *plist;
		while (*q++ = *p++)
			;
		q[-1] = ' ';
	}
	q[-1] = 0;

	return (ret);
}
