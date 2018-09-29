/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/printlist.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"

#include "lp.h"

#define	DFLT_PREFIX	0
#define	DFLT_SUFFIX	0
#define	DFLT_SEP	"\n"
#define	DFLT_NEWLINE	"\n"

int			printlist_qsep	= 0;

static char		*print_prefix	= DFLT_PREFIX,
			*print_suffix	= DFLT_SUFFIX,
			*print_sep	= DFLT_SEP,
			*print_newline	= DFLT_NEWLINE;

static void		q_print();

/**
 ** printlist_setup() - ARRANGE FOR CUSTOM PRINTING
 ** printlist_unsetup() - RESET STANDARD PRINTING
 **/

void			printlist_setup (prefix, suffix, sep, newline)
	char			*prefix,
				*suffix,
				*sep,
				*newline;
{
	if (prefix)
		print_prefix = prefix;
	if (suffix)
		print_suffix = suffix;
	if (sep)
		print_sep = sep;
	if (newline)
		print_newline = newline;
	return;
}

void			printlist_unsetup ()
{
	print_prefix = DFLT_PREFIX;
	print_suffix = DFLT_SUFFIX;
	print_sep = DFLT_SEP;
	print_newline = DFLT_NEWLINE;
	return;
}

/**
 ** printlist() - PRINT LIST ON OPEN CHANNEL
 **/

int			printlist (fp, list)
	register FILE		*fp;
	register char		**list;
{
	register char		*sep;

	if (!list || !*list)
		return (0);

	for (sep = ""; *list; *list++, sep = print_sep) {

		FPRINTF (fp, "%s%s", sep, NB(print_prefix));
		if (printlist_qsep)
			q_print (fp, *list, print_sep);
		else
			FPRINTF (fp, "%s", *list);
		FPRINTF (fp, "%s", NB(print_suffix));
		if (ferror(fp))
			return (-1);

	}
	FPRINTF (fp, print_newline);

	return (0);
}

/**
 ** q_print() - PRINT STRING, QUOTING SEPARATOR CHARACTERS
 **/

static void		q_print (fp, str, sep)
	FILE			*fp;
	register char		*str,
				*sep;
{
	while (*str) {
		if (strchr(sep, *str))
			putc ('\\', fp);
		putc (*str, fp);
		str++;
	}
	return;
}
