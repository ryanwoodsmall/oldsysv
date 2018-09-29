/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/filtertable.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "stdio.h"
#include "string.h"

#include "lp.h"
#include "filters.h"

extern void		free();

/**
 ** get_and_load() - LOAD REGULAR FILTER TABLE
 **/

int			get_and_load ()
{
	register char		*file;

	if (!(file = getfilterfile(FILTERTABLE)))
		return (-1);
	if (loadfilters(file) == -1) {
		free (file);
		return (-1);
	}
	free (file);
	return (0);
}

/**
 ** open_filtertable()
 **/

FILE			*open_filtertable (file, mode)
	char			*file,
				*mode;
{
	int			freeit;

	FILE			*fp;

	if (!file) {
		if (!(file = getfilterfile(FILTERTABLE)))
			return (0);
		freeit = 1;
	} else
		freeit = 0;
	
	fp = open_lpfile(file, mode, MODE_READ);

	if (freeit)
		free (file);

	return (fp);
}

/**
 ** close_filtertable()
 **/

void			close_filtertable (fp)
	FILE			*fp;
{
	(void)close_lpfile (fp);
	return;
}
