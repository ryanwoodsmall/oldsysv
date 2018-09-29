/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/default.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"

#include "lp.h"

extern void		free();

/**
 ** getdefault() - READ THE NAME OF THE DEFAULT DESTINATION FROM DISK
 **/

char			*getdefault ()
{
	register char		*path,
				*ret;

	if (!(path = makepath(getspooldir(), DEFAULTFILE, (char *)0)))
		return (0);

	if (!(ret = loadline(path))) {
		free (path);
		return (0);
	}
	free (path);
	return (ret);
}

/**
 ** putdefault() - WRITE THE NAME OF THE DEFAULT DESTINATION TO DISK
 **/

int			putdefault (dflt)
	char			*dflt;
{
	register char		*path;

	register FILE		*fp;

	if (!dflt || !*dflt)
		return (deldefault());

	if (!(path = makepath(getspooldir(), DEFAULTFILE, (char *)0)))
		return (-1);

	if (!(fp = open_lpfile(path, "w", MODE_READ))) {
		free (path);
		return (-1);
	}
	free (path);

	fprintf (fp, "%s\n", dflt);

	close_lpfile (fp);
	return (0);
}

/**
 ** deldefault() - REMOVE THE NAME OF THE DEFAULT DESTINATION
 **/

int			deldefault ()
{
	register char		*path;

	if (!(path = makepath(getspooldir(), DEFAULTFILE, (char *)0)))
		return (-1);

	if (rmfile(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	return (0);
}
