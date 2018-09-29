/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/which.c	1.5"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "ctype.h"
#include "string.h"
#include "unistd.h"

#include "lp.h"

extern void		free();

/**
 ** isprinter() - SEE IF ARGUMENT IS A REAL PRINTER
 **/

int			isprinter (str)
	char			*str;
{
	char			*path	= 0;

	int			bool;

	bool = (
		str
	     && *str
	     && (path = getprinterfile(str, CONFIGFILE))
	     && Access(path, F_OK) == 0
	);
	if (path)
		free (path);
	return (bool);
}

/**
 ** isclass() - SEE IF ARGUMENT IS A REAL CLASS
 **/

int			isclass (str)
	char			*str;
{
	char			*path	= 0;

	int			bool;

	bool = (
		str
	     && *str
	     && (path = getclassfile(str))
	     && Access(path, F_OK) == 0
	);
	if (path)
		free (path);
	return (bool);
}

/**
 ** isrequest() - SEE IF ARGUMENT LOOKS LIKE A REAL REQUEST
 **/

int			isrequest (str)
	char			*str;
{
	char			*dashp;

	/*
	 * Valid print requests have the form
	 *
	 *	dest-NNN
	 *
	 * where ``dest'' looks like a printer or class name.
	 * An earlier version of this routine checked to see if
	 * the ``dest'' was an EXISTING printer or class, but
	 * that caused problems with valid requests moved from
	 * a deleted printer or class (the request ID doesn't
	 * change in the new LP).
	 */

	if (!str || !*str)
		return (0);

	if (!(dashp = strchr(str, '-')))
		return (0);

	if (dashp == str)
	    return(0);

	*dashp = 0;
	if (!syn_name(str)) {
		*dashp = '-';
		return (0);
	}
	*dashp++ = '-';

	if (!isnumber(dashp))
		return (0);

	return (1);
}

int			isnumber (s)
	char			*s;
{
	register int		c;

	if (!s || !*s)
		return (0);
	while ((c = *(s++)) != '\0')
		if (!isdigit(c))
			return (0);
	return (1);
}
