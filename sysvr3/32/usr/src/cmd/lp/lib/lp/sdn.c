/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/sdn.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"

#include "lp.h"

/**
 ** printsdn() - PRINT A SCALED DECIMAL NUMBER NICELY
 **/

#define	DFLT_PREFIX	0
#define	DFLT_SUFFIX	0
#define	DFLT_NEWLINE	"\n"

static char		*print_prefix	= DFLT_PREFIX,
			*print_suffix	= DFLT_SUFFIX,
			*print_newline	= DFLT_NEWLINE;

void			printsdn_setup (prefix, suffix, newline)
	char			*prefix,
				*suffix,
				*newline;
{
	if (prefix)
		print_prefix = prefix;
	if (suffix)
		print_suffix = suffix;
	if (newline)
		print_newline = newline;
	return;
}

void			printsdn_unsetup ()
{
	print_prefix = DFLT_PREFIX;
	print_suffix = DFLT_SUFFIX;
	print_newline = DFLT_NEWLINE;
	return;
}

void			printsdn (fp, sdn)
	FILE			*fp;
	SCALED			sdn;
{
	register char		*dec = "9999.999",
				*z;

	if (sdn.val <= 0)
		return;

	FPRINTF (fp, "%s", NB(print_prefix));

	/*
	 * Let's try to be a bit clever in dealing with decimal
	 * numbers. If the number is an integer, don't print
	 * a decimal point. If it isn't an integer, strip trailing
	 * zeros from the fraction part, and don't print more
	 * than the thousandths place.
	 */
	if (-1000. < sdn.val && sdn.val < 10000.) {

		/*
		 * Printing 0 will give us 0.000.
		 */
		sprintf (dec, "%.3f", sdn.val);

		/*
		 * Skip zeroes from the end until we hit
		 * '.' or not-0. If we hit '.', clobber it;
		 * if we hit not-0, it has to be in fraction
		 * part, so leave it.
		 */
		z = dec + strlen(dec) - 1;
		while (*z == '0' && *z != '.')
			z--;
		if (*z == '.')
			*z = '\0';
		else
			*++z = '\0';

		FPRINTF (fp, "%s", dec);

	} else
		FPRINTF (fp, "%.3f", sdn.val);

	FPRINTF (
		fp,
		"%s",
		WHEN sdn.sc == 'i'
		USE "i"
		OTHERWISE
			WHEN sdn.sc == 'c'
			USE "c"
			OTHERWISE ""
			NEHW
		NEHW
	);

	FPRINTF (fp, "%s%s", NB(print_suffix), NB(print_newline));
	return;
}

