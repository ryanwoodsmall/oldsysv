/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/form.c	1.6"

#include "stdio.h"

#include "string.h"

#include "lp.h"
#include "form.h"
#include "access.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


static void		putfline();

/**
 ** do_form()
 **/

void			do_form (list)
	char			**list;
{
	FORM			form;

	while (*list) {
		if (STREQU(NAME_ALL, *list))
			while (getform(NAME_ALL, &form, (FALERT *)0, (FILE **)0) != -1)
				putfline (&form);

		else if (getform(*list, &form, (FALERT *)0, (FILE **)0) != -1) {
#define	NAME_BUG_IN_GETFORM
#if	defined(NAME_BUG_IN_GETFORM)
			form.name = *list;
#endif
			putfline (&form);

		} else {
			LP_ERRMSG1 (ERROR, E_LP_NOFORM, *list);
			exit_rc = 1;
		}

		list++;
	}
	printsdn_unsetup ();
	return;
}

static void		putfline (pf)
	FORM			*pf;
{
	register MOUNTED	*pm;


	PRINTF ("form %s", pf->name);

	PRINTF (
		" is %s to you",
is_user_allowed_form(getname(), pf->name) ? "available" : "not available"
	);

	for (pm = mounted_forms; pm->forward; pm = pm->forward)
		if (STREQU(pm->name, pf->name)) {
			if (pm->printers) {
				PRINTF (", mounted on ");
				printlist_setup (0, 0, ",", "");
				printlist (stdout, pm->printers);
				printlist_unsetup();
			}
			break;
		}

	PRINTF ("\n");

	if (verbose) {

		printsdn_setup ("\tPage length: ", 0, 0);
		printsdn (stdout, pf->plen);

		printsdn_setup ("\tPage width: ", 0, 0);
		printsdn (stdout, pf->pwid);

		PRINTF ("\tNumber of pages: %d\n", pf->np);

		printsdn_setup ("\tLine pitch: ", 0, 0);
		printsdn (stdout, pf->lpi);

		PRINTF ("\tCharacter pitch:");
		if (pf->cpi.val == 9999)
			PRINTF (" compressed\n");
		else {
			printsdn_setup (" ", 0, 0);
			printsdn (stdout, pf->cpi);
		}

		PRINTF (
			"\tCharacter set choice: %s%s\n",
			(pf->chset? pf->chset : NAME_ANY),
			(pf->mandatory ? ",mandatory" : "")
		);

		PRINTF (
			"\tRibbon color: %s\n",
			(pf->rcolor? pf->rcolor : NAME_ANY)
		);

		if (pf->comment)
			PRINTF ("\tComment:\n\t%s\n", pf->comment);

	}
	return;
}

