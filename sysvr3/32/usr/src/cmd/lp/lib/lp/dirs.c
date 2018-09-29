/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/dirs.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "fs.h"
#include "lp.h"

/**
 ** mkdir_lpdir()
 **/

int			mkdir_lpdir (path, mode)
	char			*path;
	int			mode;
{
	if (Mkdir(path, mode) != -1)
		return (chown_lppath(path));
	else
		return (-1);
}
