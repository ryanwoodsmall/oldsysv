/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/syntax.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "ctype.h"
#include "string.h"

#include "lp.h"

int			syn_name (str)
	char			*str;
{
	register char		*p;

	if (!*str)
	  	return(0);

	if (strlen(str) > 14)
		return (0);

	for (p = str; *p; p++)
		if (!isalnum(*p) && *p != '_')
			return (0);

	return (1);
}

int			syn_type (str)
	char			*str;
{
	register char		*p;

	if (strlen(str) > 14)
		return (0);

	for (p = str; *p; p++)
		if (!isalnum(*p) && *p != '-')
			return (0);

	return (1);
}

int			syn_text (str)
	char			*str;
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isgraph(*p) && *p != '\t' && *p != ' ')
			return (0);

	return (1);
}

int			syn_comment (str)
	char			*str;
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isgraph(*p) && *p != '\t' && *p != ' ' && *p != '\n')
			return (0);

	return (1);
}

int			syn_machine_name (str)
	char			*str;
{
	if (strlen(str) > 8)
		return (0);

	return (1);
}

int			syn_option (str)
	char			*str;
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isprint(*p))
			return (0);

	return (1);
}
