/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/access/dumpaccess.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"

#include "lp.h"
#include "access.h"

extern void		free();

static int		_dumpaccess();

/**
 ** dumpaccess() - DUMP ALLOW OR DENY LISTS
 **/

int			dumpaccess (dir, name, prefix, pallow, pdeny)
	char			*dir,
				*name,
				*prefix,
				***pallow,
				***pdeny;
{
	register char		*allow_file	= 0,
				*deny_file	= 0;

	int			ret;

	if (
		!(allow_file = getaccessfile(dir, name, prefix, "allow"))
	     || _dumpaccess(allow_file, *pallow) == -1 && errno != ENOENT
	     || !(deny_file = getaccessfile(dir, name, prefix, "deny"))
	     || _dumpaccess(deny_file, *pdeny) == -1 && errno != ENOENT
	)
		ret = -1;
	else
		ret = 0;

	if (allow_file)
		free (allow_file);
	if (deny_file)
		free (deny_file);

	return (ret);
}

/**
 ** _dumpaccess() - DUMP ALLOW OR DENY FILE
 **/

static int		_dumpaccess (file, list)
	char			*file,
				**list;
{
	register char		**pl;

	register int		ret;

	FILE			*fp;

	if (list) {
		if (!(fp = open_lpfile(file, "w", MODE_READ)))
			return (-1);
		for (pl = list; *pl; pl++)
			fprintf (fp, "%s\n", *pl);
		if (ferror(fp))
			ret = -1;
		else
			ret = 0;
		close_lpfile (fp);
	} else
		ret = Unlink(file);

	return (ret);
}
