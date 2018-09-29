/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/charset.c	1.3"

#include "string.h"
#include "sys/types.h"

#include "lp.h"
#include "form.h"
#include "access.h"
#include "printers.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


extern char		*tparm(),
			*malloc();

static void		putsline();

/**
 ** do_charset()
 **/

void			do_charset (list)
	char			**list;
{
	register MOUNTED	*pm;

	register int		found;


	while (*list) {
		if (STREQU(NAME_ALL, *list))
			for (pm = mounted_pwheels; pm->name; pm = pm->forward)
				putsline (pm);

		else {
			found = 0;
			for (pm = mounted_pwheels; pm->name; pm = pm->forward)
				if (
					pm->name[0] == '!'
				     && STREQU(pm->name + 1, *list)
				     || STREQU(pm->name, *list)
				) {
					putsline (pm);
					found = 1;
				}
			if (!found) {
				LP_ERRMSG1 (ERROR, E_STAT_BADSET, *list);
				exit_rc = 1;
			}
		}
		list++;
	}
	return;
}

static void		putsline (pm)
	register MOUNTED	*pm;
{
	register char		**pp,
				*sep;


	if (pm->name[0] != '!') {

		PRINTF ("print wheel %s", pm->name);

		if ((pp = pm->printers))
			if (verbose) {
				PRINTF ("\n\tavailable on:");
				while (*pp) {
					if ((*pp)[0] == '!')
						PRINTF (
							"\n\t\t%s (mounted)",
							*pp + 1
						);
					else
						PRINTF ("\n\t\t%s", *pp);
					pp++;
				}
			} else {
				sep = ", mounted on ";
				while (*pp) {
					if (verbose || (*pp)[0] == '!') {
						PRINTF (
							"%s%s",
							sep,
					((*pp)[0] == '!'? *pp + 1 : *pp)
						);
						sep = ",";
					}
					pp++;
				}
			}

		PRINTF ("\n");

	} else {

		PRINTF ("character set %s\n", pm->name + 1);

		if (verbose && (pp = pm->printers)) {
			PRINTF ("\tavailable on:\n");
			while (*pp) {
				PRINTF (
					"\t\t%s (as %s)\n",
					strtok(*pp, "="),
					strtok((char *)0, "=")
				);
				pp++;
			}
		}

	}
	return;
}

/**
 ** get_charsets() - CONSTRUCT (char **) LIST OF CHARSETS FROM csnm
 **/

char			**get_charsets (prbufp, addcs)
	PRINTER			*prbufp;
	register int		addcs;
{
	char			*csnm,
				**list		= 0;

	register int		cs		= 0;

	register char		*name;


	if (
		prbufp->printer_type
	     && !STREQU(prbufp->printer_type, NAME_UNKNOWN)
	     && !prbufp->daisy
	     && tidbit(prbufp->printer_type, "csnm", &csnm) != -1
	     && csnm
	     && *csnm
	) {
		for (cs = 0; cs <= 63; cs++)
			if ((name = tparm(csnm, cs)) && *name) {

				if (addcs) {
					register char	 *nm = malloc(
						5 + strlen(name) + 1
					);

					if (nm) {
						sprintf (
							nm,
							"cs%d=%s",
							cs,
							name
						);
						name = nm;
					}
				}

				if (addlist(&list, name) == -1) {
					LP_ERRMSG (ERROR, E_LP_MALLOC);
					done (1);
				}

			} else
				/*
				 * Assume that a break in the
				 * numbers means we're done.
				 */
				break;
	}

	return (list);
}
