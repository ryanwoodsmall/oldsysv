/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/access/files.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

#include "lp.h"

extern char		*malloc();

extern void		free();

/**
 ** getaccessfile() - BUILD NAME OF ALLOW OR DENY FILE
 **/

char			*getaccessfile (dir, name, prefix, base)
	char			*dir,
				*name,
				*prefix,
				*base;
{
	register char		*spooldir,
				*parent,
				*file,
				*f;

	if (!(spooldir = getspooldir()))
		return (0);

	/*
	 * It makes no sense talking about the access files if
	 * the directory for the form or printer doesn't exist.
	 */
	parent = makepath(spooldir, ADMINSDIR, LPUSER, dir, name, (char *)0);
	if (!parent)
		return (0);
	if (Access(parent, F_OK) == -1)
		return (0);

	if (!(f = malloc(strlen(prefix) + strlen(base) + 1))) {
		errno = ENOMEM;
		return (0);
	}
	strcpy (f, prefix);
	strcat (f, base);

	file = makepath(parent, f, (char *)0);
	free (f);
	free (parent);

	return (file);
}
