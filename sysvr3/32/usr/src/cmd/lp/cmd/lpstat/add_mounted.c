/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/add_mounted.c	1.5"

#include "string.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "printers.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


extern char		*sys_errlist[];

extern int		sys_nerr;

extern char		*malloc(),
			*realloc();

static MOUNTED		mounted_forms_head	= { 0, 0, 0 },
			mounted_pwheels_head	= { 0, 0, 0 };

MOUNTED			*mounted_forms		= &mounted_forms_head,
			*mounted_pwheels	= &mounted_pwheels_head;

static MOUNTED		*nuther_mount();

static void		insert_mounted();

static int		cs_addlist();

/**
 ** add_mounted() - ADD A FORM, PRINTWHEEL TO LIST
 **/

void			add_mounted (printer, form, pwheel)
	char			*printer,
				*form,
				*pwheel;
{
	register PRINTER	*pp;

	register char		**pcs;


	/*
	 * Add this form to the list of mounted forms.
	 */
	if (form && *form)
		insert_mounted (
			mounted_forms,
			form,
			printer,
			(char *)0
		);

	/*
	 * Add this print wheel to the list of mounted print
	 * wheels.
	 */
	if (pwheel && *pwheel)
		insert_mounted (
			mounted_pwheels,
			pwheel,
			printer,
			"!"
		);

	if (!(pp = getprinter(printer))) {
		LP_ERRMSG2 (
			ERROR,
			E_LP_GETPRINTER,
			printer,
			(errno < sys_nerr? sys_errlist[errno]: "unknown")
		);
		done(1);
	}

	/*
	 * Add the list of administrator supplied character-set
	 * or print wheel aliases to the list.
	 */
	if (pp->char_sets)
		for (pcs = pp->char_sets; *pcs; pcs++)
			if (pp->daisy)
				insert_mounted (
					mounted_pwheels,
					*pcs,
					printer,
					(char *)0
				);

			else {
				register char		*terminfo_name,
							*alias;

				if (
					(terminfo_name = strtok(*pcs, "="))
				     && (alias = strtok((char *)0, "="))
				)
					insert_mounted (
						mounted_pwheels,
						alias,
						printer,
						terminfo_name
					);
			}

	/*
	 * Do the aliases built into the Terminfo database for
	 * this printer's type. This applies only to printers
	 * that have defined character sets.
	 */
	if ((pcs = get_charsets(pp, 1)))
		for ( ; *pcs; pcs++) {
			register char		*terminfo_name,
						*alias;

			if (
				(terminfo_name = strtok(*pcs, "="))
			     && (alias = strtok((char *)0, "="))
			)
				insert_mounted (
					mounted_pwheels,
					alias,
					printer,
					terminfo_name
				);
		}


	return;
}

static void		insert_mounted (ml, name, printer, tag)
	MOUNTED			*ml;
	char			*name,
				*printer,
				*tag;
{
	register MOUNTED	*pm;

	register char		*nm,
				*pnm;


	/*
	 * For forms: Each printer that has the form mounted is
	 * marked with a leading '!'.
	 * For print wheels and character sets: Each character
	 * set name is marked with a leading '!'; each printer
	 * that has a print wheel mounted is marked with a leading '!'.
	 */

	if (tag && tag[0] != '!') {
		if (!(nm = malloc(1 + strlen(name) + 1))) {
			LP_ERRMSG (ERROR, E_LP_MALLOC);
			done (1);
		}
		nm[0] = '!';
		(void)strcpy (nm + 1, name);
	} else
		nm = name;

	for (
		pm = ml;
		pm->name && !STREQU(pm->name, nm);
		pm = pm->forward
	)
		;

	if (tag && tag[0] == '!') {
		if (!(pnm = malloc(1 + strlen(printer) + 1))) {
			LP_ERRMSG (ERROR, E_LP_MALLOC);
			done (1);
		}
		pnm[0] = '!';
		(void)strcpy (pnm + 1, printer);

	} else if (tag) {
		if (!(pnm = malloc(strlen(printer) + 1 + strlen(tag) + 1))) {
			LP_ERRMSG (ERROR, E_LP_MALLOC);
			done (1);
		}
		SPRINTF (pnm, "%s=%s", printer, tag);

	} else
		pnm = printer;


	if (cs_addlist(&(pm->printers), pnm) == -1) {
		LP_ERRMSG (ERROR, E_LP_MALLOC);
		done (1);
	}

	if (!pm->name) {
		pm->name = strdup(nm);
		pm = (pm->forward = nuther_mount());
		pm->name = 0;
		pm->printers = 0;
		pm->forward = 0;
	}

	return;
}

/**
 ** nuther_mount() - ALLOCATE ANOTHER (MOUNTED) ELEMENT
 **/

static MOUNTED		*nuther_mount ()
{
	register MOUNTED	*pm;

	if (!(pm = (MOUNTED *)malloc(sizeof(MOUNTED)))) {
		LP_ERRMSG (ERROR, E_LP_MALLOC);
		done (1);
	}
	return (pm);
}

static int		cs_addlist (plist, item)
	register char		***plist,
				*item;
{
	register char		**pl;

	register int		n;

	if (*plist) {

		n = lenlist(*plist);

		for (pl = *plist; *pl; pl++)
			if (
				(*pl)[0] == '!' && STREQU((*pl)+1, item)
			     || STREQU(*pl, item)
			)
				break;

		if (!*pl) {

			n++;
			*plist = (char **)realloc(
				(char *)*plist,
				(n + 1) * sizeof(char *)
			);
			if (!*plist) {
				errno = ENOMEM;
				return (-1);
			}
			(*plist)[n - 1] = strdup(item);
			(*plist)[n] = 0;

		}

	} else {

		*plist = (char **)malloc(2 * sizeof(char *));
		if (!*plist) {
			errno = ENOMEM;
			return (-1);
		}
		(*plist)[0] = strdup(item);
		(*plist)[1] = 0;

	}

	return (0);
}
