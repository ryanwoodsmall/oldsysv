/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/access/loadaccess.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "errno.h"
#include "string.h"

#include "lp.h"
#include "access.h"

extern char		*malloc(),
			*realloc();

extern void		free();

static char		**_loadaccess();

/**
 ** load_userform_access() - LOAD ALLOW/DENY LISTS FOR USER+FORM
 **/

int			load_userform_access (form, pallow, pdeny)
	char			*form,
				***pallow,
				***pdeny;
{
	return (loadaccess(FORMSDIR, form, "", pallow, pdeny));
}

/**
 ** load_userprinter_access() - LOAD ALLOW/DENY LISTS FOR USER+PRINTER
 **/

int			load_userprinter_access (printer, pallow, pdeny)
	char			*printer,
				***pallow,
				***pdeny;
{
	return (loadaccess(PRINTERSDIR, printer, "users.", pallow, pdeny));
}

/**
 ** load_formprinter_access() - LOAD ALLOW/DENY LISTS FOR FORM+PRINTER
 **/

int			load_formprinter_access (printer, pallow, pdeny)
	char			*printer,
				***pallow,
				***pdeny;
{
	return (loadaccess(PRINTERSDIR, printer, "forms.", pallow, pdeny));
}

/**
 ** loadaccess() - LOAD ALLOW OR DENY LISTS
 **/

int			loadaccess (dir, name, prefix, pallow, pdeny)
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
	     || !(*pallow = _loadaccess(allow_file)) && errno != ENOENT
	     || !(deny_file = getaccessfile(dir, name, prefix, "deny"))
	     || !(*pdeny = _loadaccess(deny_file)) && errno != ENOENT
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
 ** _loadaccess() - LOAD ALLOW OR DENY FILE
 **/

static char		**_loadaccess (file)
	char			*file;
{
	register int		nalloc,
				nlist;

	register char		**list;

	FILE			*fp;

	char			buf[BUFSIZ];


	if (!(fp = open_lpfile(file, "r")))
		return (0);

	/*
	 * Preallocate space for the initial list. We'll always
	 * allocate one more than the list size, for the terminating null.
	 */
	nalloc = ACC_MAX_GUESS;
	list = (char **)malloc((nalloc + 1) * sizeof(char *));
	if (!list) {
		close_lpfile (fp);
		errno = ENOMEM;
		return (0);
	}

	for (nlist = 0; fgets(buf, BUFSIZ, fp); ) {

		buf[strlen(buf) - 1] = 0;

		/*
		 * Allocate more space if needed.
		 */
		if (nlist >= nalloc) {
			nalloc += ACC_MAX_GUESS;
			list = (char **)realloc(
				(char *)list,
				(nalloc + 1) * sizeof(char *)
			);
			if (!list) {
				close_lpfile (fp);
				return (0);
			}
		}

		list[nlist] = strdup(buf);   /* if fail, minor problem */
		list[++nlist] = 0;

	}
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		freelist (list);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	/*
	 * If we have more space allocated than we need,
	 * return the extra.
	 */
	if (nlist != nalloc) {
		list = (char **)realloc(
			(char *)list,
			(nlist + 1) * sizeof(char *)
		);
		if (!list) {
			errno = ENOMEM;
			return (0);
		}
	}
	list[nlist] = 0;

	return (list);
}
