/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/dumpfilters.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"

#include "lp.h"
#include "filters.h"

static void		q_print();

/**
 ** dumpfilters() - WRITE FILTERS FROM INTERNAL STRUCTURE TO FILTER TABLE
 **/

int			dumpfilters (file)
	char			*file;
{
	register _FILTER	*pf;

	register TEMPLATE	*pt;

	register TYPE		*pty;

	register char		*p,
				*sep;

	register int		fld;

	FILE			*fp;

	if (!(fp = open_filtertable(file, "w")))
		return (-1);

	printlist_setup ("", "", LP_SEP, "");
	if (filters) for (pf = filters; pf->name; pf++) {

		for (fld = 0; fld < FL_MAX; fld++) switch (fld) {
		case FL_IGN:
			break;
		case FL_NAME:
			p = pf->name;
			goto String;
		case FL_CMD:
			p = pf->command;
String:			FPRINTF (fp, "%s%s", FL_SEP, (p? p : ""));
			break;
		case FL_TYPE:
			FPRINTF (
				fp,
				"%s%s",
				FL_SEP,
				(pf->type == fl_fast? FL_FAST : FL_SLOW)
			);
			break;
		case FL_PTYPS:
			pty = pf->printer_types;
			goto Types;
		case FL_ITYPS:
			pty = pf->input_types;
			goto Types;
		case FL_OTYPS:
			pty = pf->output_types;
Types:			FPRINTF (fp, "%s", FL_SEP);
			sep = "";
			if (pty) {
				for (; pty->name; pty++) {
					FPRINTF (
						fp,
						"%s%s",
						sep,
						pty->name
					);
					sep = ",";
				}
			} else
				FPRINTF (fp, "%s", NAME_ANY);
			break;
		case FL_PRTRS:
			FPRINTF (fp, "%s", FL_SEP);
			if (pf->printers)
				printlist (fp, pf->printers);
			else
				FPRINTF (fp, "%s", NAME_ANY);
			break;
		case FL_TMPS:
			FPRINTF (fp, "%s", FL_SEP);
			sep = "";
			if ((pt = pf->templates))
				for(; pt->keyword; pt++) {
					FPRINTF (
						fp,
						"%s%s ",
						sep,
						pt->keyword
					);
					q_print (fp, pt->pattern);
					FPRINTF (fp, " = ");
					q_print (fp, pt->result);
					sep = ",";
				}
			break;
		}
		FPRINTF (fp, FL_END);
	}

	close_filtertable (fp);
	return (0);
}

/**
 ** q_print() - PRINT STRING, QUOTING SEPARATOR CHARACTERS
 **/

static void		q_print (fp, str)
	register FILE		*fp;
	register char		*str;
{
	/*
	 * There are four reasons to quote a character: It is
	 * a quote (backslash) character, it is a field separator,
	 * it is a list separator, or it is a template separator.
	 * "loadfilters()" strips the quote (backslash), but not
	 * in one place.
	 */
	while (*str) {
		if (
			*str == '\\'		/* quote reason #1 */
		     || strchr(FL_SEP, *str)	/* quote reason #2 */
		     || strchr(LP_SEP, *str)	/* quote reason #3 */
		     || strchr("=", *str)	/* quote reason #4 */
		)
			putc ('\\', fp);
		putc (*str, fp);
		str++;
	}
	return;
}
