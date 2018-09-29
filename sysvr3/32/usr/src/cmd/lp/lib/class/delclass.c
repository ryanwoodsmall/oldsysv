/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/class/delclass.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "fs.h"

extern void		free();

/**
 ** delclass() - WRITE CLASS OUT TO DISK
 **/

int			delclass (name)
	char			*name;
{
	long			lastdir;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		if (!Lp_A_Classes) {
			getadminpaths (LPUSER);
			if (!Lp_A_Classes)
				return (0);
		}
		while ((name = next_file(Lp_A_Classes, &lastdir)))
			if (_delclass(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delclass(name));
}

/**
 ** _delclass()
 **/

static int		_delclass (name)
	char			*name;
{
	char			*path;

	if (!(path = getclassfile(name)))
		return (-1);
	if (rmfile(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);
	return (0);
}

