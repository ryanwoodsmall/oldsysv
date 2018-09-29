/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/class/putclass.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "fs.h"
#include "class.h"

/**
 ** putclass() - WRITE CLASS OUT TO DISK
 **/

int			putclass (name, clsbufp)
	char			*name;
	CLASS			*clsbufp;
{
	char			*file;

	FILE			*fp;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Open the class file and write out the class members.
	 */

	if (!(file = getclassfile(name)))
		return (-1);

	if (!(fp = open_lpfile(file, "w", MODE_READ))) {
		free (file);
		return (-1);
	}
	free (file);

	printlist (fp, clsbufp->members);

	if (ferror(fp)) {
		close_lpfile (fp);
		return (-1);
	}
	close_lpfile (fp);

	return (0);
}
