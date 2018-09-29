/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/delprinter.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "fs.h"
#include "printers.h"

extern void		free();

static int		_delprinter();

/**
 ** delprinter()
 **/

int			delprinter (name)
	char			*name;
{
	long			lastdir;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (!Lp_A_Printers || !Lp_A_Interfaces) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers || !Lp_A_Interfaces)
			return (0);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		while ((name = next_dir(Lp_A_Printers, &lastdir)))
			if (_delprinter(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delprinter(name));
}

/**
 ** _delprinter()
 **/

static int		_delprinter (name)
	char			*name;
{
	register char		*path;

#define RMFILE(X)	if (!(path = getprinterfile(name, X))) \
				return (-1); \
			if (rmfile(path) == -1) { \
				free (path); \
				return (-1); \
			} \
			free (path)
	RMFILE (COMMENTFILE);
	RMFILE (CONFIGFILE);
	RMFILE (FALLOWFILE);
	RMFILE (FDENYFILE);
	RMFILE (UALLOWFILE);
	RMFILE (UDENYFILE);
	RMFILE (STATUSFILE);

	delalert (Lp_A_Printers, name);

	if (!(path = makepath(Lp_A_Interfaces, name, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	if (!(path = getprinterfile(name, (char *)0)))
		return (-1);
	if (Rmdir(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	return (0);
}
