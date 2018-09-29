/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/access/change.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"

#include "lp.h"
#include "access.h"

extern char		*malloc();

static int		chgaccess();

static char		**empty_list();

/**
 ** deny_user_form() - DENY USER ACCESS TO FORM
 **/

int			deny_user_form (user_list, form)
	char			**user_list,
				*form;
{
	return (chgaccess(0, user_list, form, FORMSDIR, ""));
}

/**
 ** allow_user_form() - ALLOW USER ACCESS TO FORM
 **/

int			allow_user_form (user_list, form)
	char			**user_list,
				*form;
{
	return (chgaccess(1, user_list, form, FORMSDIR, ""));
}

/**
 ** deny_user_printer() - DENY USER ACCESS TO PRINTER
 **/

int			deny_user_printer (user_list, printer)
	char			**user_list,
				*printer;
{
	return (chgaccess(0, user_list, printer, PRINTERSDIR, "users."));
}

/**
 ** allow_user_printer() - ALLOW USER ACCESS TO PRINTER
 **/

int			allow_user_printer (user_list, printer)
	char			**user_list,
				*printer;
{
	return (chgaccess(1, user_list, printer, PRINTERSDIR, "users."));
}

/**
 ** deny_form_printer() - DENY FORM USE ON PRINTER
 **/

int			deny_form_printer (form_list, printer)
	char			**form_list,
				*printer;
{
	return (chgaccess(0, form_list, printer, PRINTERSDIR, "forms."));
}

/**
 ** allow_form_printer() - ALLOW FORM USE ON PRINTER
 **/

int			allow_form_printer (form_list, printer)
	char			**form_list,
				*printer;
{
	return (chgaccess(1, form_list, printer, PRINTERSDIR, "forms."));
}

/**
 ** chgaccess() - UPDATE ALLOW/DENY ACCESS OF ITEM TO RESOURCE
 **/

static int		chgaccess (isallow, list, name, dir, prefix)
	int			isallow;
	char			**list,
				*name,
				*dir,
				*prefix;
{
	register char		***padd_list,
				***prem_list,
				**pl;

	char			**allow_list,
				**deny_list;

	if (loadaccess(dir, name, prefix, &allow_list, &deny_list) == -1)
		return (-1);

	if (isallow) {
		padd_list = &allow_list;
		prem_list = &deny_list;
	} else {
		padd_list = &deny_list;
		prem_list = &allow_list;
	}

	for (pl = list; *pl; pl++) {

		if (STREQU(*pl, NAME_NONE)) {

			isallow = !isallow;
			goto Allcase;

		} else if (
			STREQU(*pl, NAME_ALL)
		     || STREQU(*pl, NAME_ANY)
		) {

Allcase:		freelist (allow_list);
			freelist (deny_list);
			if (isallow) {
				allow_list = 0;
				deny_list = empty_list();
			} else {
				allow_list = 0;
				deny_list = 0;
			}
			break;

		} else {

			/*
			 * For each regular item in the list,
			 * we add it to the ``add list'' and remove it
			 * from the ``remove list''. This is not
			 * efficient, especially if there are alot of
			 * items in the caller's list; doing it the
			 * way we do, however, has the side effect
			 * of skipping duplicate names in the caller's
			 * list.
			 */
			if (addlist(padd_list, *pl) == -1)
				return (-1);
			if (dellist(prem_list, *pl) == -1)
				return (-1);

		}

	}

	return (dumpaccess(dir, name, prefix, &allow_list, &deny_list));
}

/**
 ** empty_list() - CREATE AN EMPTY LIST
 **/

static char		**empty_list ()
{
	register char		**empty;


	if (!(empty = (char **)malloc(sizeof(char *)))) {
		errno = ENOMEM;
		return (0);
	}
	*empty = 0;
	return (empty);
}
