/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/access/allowed.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "unistd.h"

#include "lp.h"
#include "access.h"

/**
 ** is_user_admin() - CHECK IF CURRENT USER IS AN ADMINISTRATOR
 **/

int			is_user_admin ()
{
	return (Access(getspooldir(), W_OK) == -1? 0 : 1);
}

/**
 ** is_user_allowed() - CHECK USER ACCESS ACCORDING TO ALLOW/DENY LISTS
 **/

int			is_user_allowed (user, allow, deny)
	char			*user,
				**allow,
				**deny;
{
	if (STREQU(user, LPUSER) || STREQU(user, ROOTUSER))
		return (1);

	return (allowed(user, allow, deny));
}

/**
 ** is_user_allowed_form() - CHECK USER ACCESS TO FORM
 **/

int			is_user_allowed_form (user, form)
	char			*user,
				*form;
{
	char			**allow,
				**deny;

	if (loadaccess(FORMSDIR, form, "", &allow, &deny) == -1)
		return (-1);

	return (is_user_allowed(user, allow, deny));
}

/**
 ** is_user_allowed_printer() - CHECK USER ACCESS TO PRINTER
 **/

int			is_user_allowed_printer (user, printer)
	char			*user,
				*printer;
{
	char			**allow,
				**deny;

	if (loadaccess(PRINTERSDIR, printer, "users.", &allow, &deny) == -1)
		return (-1);

	return (is_user_allowed(user, allow, deny));
}

/**
 ** is_form_allowed_printer() - CHECK FORM USE ON PRINTER
 **/

int			is_form_allowed_printer (form, printer)
	char			*form,
				*printer;
{
	char			**allow,
				**deny;

	if (loadaccess(PRINTERSDIR, printer, "forms.", &allow, &deny) == -1)
		return (-1);

	return (allowed(form, allow, deny));
}

/**
 ** allowed() - GENERAL ROUTINE TO CHECK ALLOW/DENY LISTS
 **/

int			allowed (item, allow, deny)
	char			*item,
				**allow,
				**deny;
{
	if (allow) {
		if (searchlist(item, allow))
			return (1);
		else
			return (0);
	}

	if (deny) {
		if (searchlist(item, deny))
			return (0);
		else
			return (1);
	}

	return (0);
}
