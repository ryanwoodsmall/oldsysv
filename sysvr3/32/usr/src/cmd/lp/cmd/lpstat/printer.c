/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/printer.c	1.7"

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "printers.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"

extern char		*sys_errlist[];

extern int		sys_nerr;

#define	FIGURE_PITCH_SIZE	/* */

#if	defined(FIGURE_PITCH_SIZE)
static void		figure_pitch_size();
#endif

static void		printallowdeny(),
			printpwheels(),
			printsets();

/**
 ** do_printer()
 **/

void			do_printer (list)
	char			**list;
{
	while (*list) {
		if (STREQU(*list, NAME_ALL)) {
			send_message (S_INQUIRE_PRINTER_STATUS, "");
			(void)output(R_INQUIRE_PRINTER_STATUS);

		} else {
			send_message (S_INQUIRE_PRINTER_STATUS, *list);
			switch (output(R_INQUIRE_PRINTER_STATUS)) {
			case MNODEST:
				LP_ERRMSG1 (ERROR, E_LP_NOPRINTER, *list);
				exit_rc = 1;
				break;
			}
		}
		list++;
	}
	return;
}

/**
 ** putpline() - DISPLAY STATUS OF PRINTER
 **/

void			putpline (
				printer,
				printer_status,
				request_id,
				enable_date,
				disable_reason,
				form,
				character_set
			)
	int			printer_status;
	char			*printer,
				*request_id,
				*enable_date,
				*disable_reason,
				*form,
				*character_set;
{
	register PRINTER	*prbufp;

	char			**u_allow	= 0,
				**u_deny	= 0,
				**f_allow	= 0,
				**f_deny	= 0;


	if (!(prbufp = getprinter(printer))) {
		LP_ERRMSG2 (
			ERROR,
			E_LP_GETPRINTER,
			printer,
			(errno < sys_nerr? sys_errlist[errno]: "unknown")
		);
		done(1);
	}

	PRINTF ("printer %s", printer);

	if (prbufp->login)
		PRINTF (" (login terminal)");

	if (!(printer_status & (PS_DISABLED|PS_FAULTED|PS_LATER))) {
		if (printer_status & PS_BUSY)
			PRINTF (" now printing %s.", request_id);
		else
			PRINTF (" is idle.");

		PRINTF (" enabled since %.24s.", enable_date);

	} else if (printer_status & PS_DISABLED)
		PRINTF (" disabled since %.24s.", enable_date);

	else if (printer_status & PS_LATER)
		PRINTF (" waiting for auto-retry.");

	(void)load_userprinter_access (printer, &u_allow, &u_deny);
	PRINTF (
		" %s.\n",
		is_user_allowed(getname(), u_allow, u_deny)?
			  "available"
			: "not available"
	);

	if (printer_status & (PS_DISABLED|PS_LATER))
		PRINTF ("\t%s\n", disable_reason);

	if (D && !verbose)
		PRINTF ("\tDescription: %s\n", NB(prbufp->description));

	else if (verbose) {

		PRINTF ("\tForm mounted: %s\n", NB(form));


		PRINTF ("\tContent types:");
		if (prbufp->input_types) {
			printlist_setup (" ", 0, ",", "");
			printlist (stdout, prbufp->input_types);
			printlist_unsetup ();
		} else
			PRINTF (" (%s)", NAME_SIMPLE);
		PRINTF ("\n");


		PRINTF ("\tPrinter type: %s\n", NB(prbufp->printer_type));

		PRINTF ("\tDescription: %s\n", NB(prbufp->description));

		PRINTF (
			"\tConnection: %s\n",
			(prbufp->dial_info? prbufp->dial_info : NAME_DIRECT)
		);

		PRINTF ("\tInterface: %s\n", NB(prbufp->interface));

		PRINTF ("\t");
		printalert (stdout, &(prbufp->fault_alert), 1);

		PRINTF (
			"\tAfter fault: %s\n",
		(prbufp->fault_rec? prbufp->fault_rec : NAME_CONTINUE)
		);

		(void)load_formprinter_access (printer, &f_allow, &f_deny);
		printallowdeny (stdout, "\tUsers ", 0, u_allow, u_deny);
		printallowdeny (stdout, "\tForms ", 0, f_allow, f_deny);

		PRINTF (
			"\tBanner %srequired\n",
			(prbufp->banner & BAN_ALWAYS ? "" : "not ")
		);
	
		if (prbufp->daisy) {
			PRINTF ("\tPrint wheels:\n");
			printpwheels (prbufp, character_set);
		} else {
			PRINTF ("\tCharacter sets:\n");
			printsets (prbufp);
		}

#if	defined(FIGURE_PITCH_SIZE)
		figure_pitch_size (prbufp);
#endif

		PRINTF ("\tDefault pitch:");
		if (prbufp->cpi.val == 9999)
			PRINTF (" compressed CPI");
		else {
			printsdn_setup (" ", " CPI", "");
			printsdn (stdout, prbufp->cpi);
		}
		printsdn_setup (" ", " LPI", "");
		printsdn (stdout, prbufp->lpi);
		PRINTF ("\n");

		PRINTF ("\tDefault page size:");
		printsdn_setup (" ", " wide", "");
		printsdn (stdout, prbufp->pwid);
		printsdn_setup (" ", " long", "");
		printsdn (stdout, prbufp->plen);
		PRINTF ("\n");

		printsdn_unsetup ();

		PRINTF ("\tDefault port settings: %s ", NB(prbufp->stty)); 
		if (prbufp->speed && prbufp->dial_info)
			if (!STREQU(prbufp->dial_info, NAME_DIRECT))
				PRINTF ("%s\n", NB(prbufp->speed));

		PRINTF ("\n");

	} else


	return;
}

/**
 ** figure_pitch_size() - CALCULATE *REAL* DEFAULT PITCH, PAGE SIZE
 **/

#if	defined(FIGURE_PITCH_SIZE)

static void		figure_pitch_size (prbufp)
	register PRINTER	*prbufp;
{
	short			orc,
				orhi,
				orl,
				orvi,
				cols,
				lines;

	/*
	 * The user want's to know how the page will look if
	 * he or she uses this printer. Thus, if the administrator
	 * hasn't set any defaults, figure out what they are from
	 * the Terminfo entry.
	 */
	if (
		!prbufp->printer_type
	     || STREQU(prbufp->printer_type, NAME_UNKNOWN)
	)
		return;

	/*
	 * NOTE: We should never get a failure return unless
	 * someone has trashed the printer configuration file.
	 * Also, if we don't fail the first time, we can't fail
	 * subsequently.
	 */
	if (tidbit(prbufp->printer_type, "orc", &orc) == -1)
		return;
	(void)tidbit (prbufp->printer_type, "orhi", &orhi);
	(void)tidbit (prbufp->printer_type, "orl", &orl);
	(void)tidbit (prbufp->printer_type, "orvi", &orvi);
	(void)tidbit (prbufp->printer_type, "cols", &cols);
	(void)tidbit (prbufp->printer_type, "lines", &lines);

#define COMPUTE(OR,ORI) \
	(ORI != -1 && OR != -1? (int)((ORI / (double)OR) + .5) : 0)

	if (prbufp->cpi.val <= 0) {
		prbufp->cpi.val = (float)COMPUTE(orhi, orc);
		prbufp->cpi.sc = 0;
	}
	if (prbufp->lpi.val <= 0) {
		prbufp->lpi.val = (float)COMPUTE(orvi, orl);
		prbufp->lpi.sc = 0;
	}
	if (prbufp->pwid.val <= 0) {
		prbufp->pwid.val = (float)cols;
		prbufp->pwid.sc = 0;
	}
	if (prbufp->plen.val <= 0) {
		prbufp->plen.val = (float)lines;
		prbufp->plen.sc = 0;
	}

	return;
}

#endif

/**
 ** printallowdeny() - PRINT ALLOW/DENY LIST NICELY
 **/

static void		printallowdeny (fp, prefix, suffix, allow, deny)
	FILE			*fp;
	char			*prefix,
				*suffix,
				**allow,
				**deny;
{
#define	PRT(X) FPRINTF (fp, "%s%s%s:\n", NB(prefix), X, NB(suffix))

	printlist_setup ("\t\t", 0, 0, 0);

	if (allow || deny && !*deny || !deny) {
		PRT ("allowed");
		if (allow && *allow)
			printlist (fp, allow);
		else if (allow && !*allow || !deny)
			FPRINTF (fp, "\t\t(%s)\n", NAME_NONE);
		else
			FPRINTF (fp, "\t\t(%s)\n", NAME_ALL);

	} else {
		PRT ("denied");
		printlist (fp, deny);

	}

	printlist_unsetup ();
	return;
}

/**
 ** printpwheels() - PRINT LIST OF PRINT WHEELS
 **/

static void		printpwheels (prbufp, pwheel)
	register PRINTER	*prbufp;
	register char		*pwheel;
{
	register char		**list;

	register int		mount_in_list	= 0,
				something_shown	= 0;


	if ((list = prbufp->char_sets))
		while (*list) {
			PRINTF ("\t\t%s", *list);
			if (pwheel && STREQU(*list, pwheel)) {
				PRINTF (" (mounted)");
				mount_in_list = 1;
			}
			PRINTF ("\n");
			list++;
			something_shown = 1;
		}

	if (!mount_in_list && pwheel && *pwheel) {
		PRINTF ("\t\t%s (mounted)\n", pwheel);
		something_shown = 1;
	}

	if (!something_shown)
		PRINTF ("\t\t(none)\n");

	return;
}

/**
 ** printsets() - PRINT LIST OF CHARACTER SETS, WITH MAPPING
 **/

static void		printsets (prbufp)
	register PRINTER	*prbufp;
{
	register char		**alist		= prbufp->char_sets,
				*cp;

	char			**tlist = 0;


	/*
	 * We'll report the administrator defined character set aliases
	 * and any OTHER character sets we find in the Terminfo database.
	 */
	tlist = get_charsets(prbufp, 0);

	if ((!alist || !*alist) && (!tlist || !*tlist)) {
		PRINTF ("\t\t(none)\n");
		return;
	}

	if (alist)
		while (*alist) {
			cp = strchr(*alist, '=');
			if (cp)
				*cp++ = 0;

			/*
			 * Remove the alias from the Terminfo list so
			 * we don't report it twice.
			 */
			if (dellist(&tlist, *alist) == -1) {
				LP_ERRMSG (ERROR, E_LP_MALLOC);
				done (1);
			}

			if (cp)
				PRINTF ("\t\t%s (as %s)\n", cp, *alist);
			else
				PRINTF ("\t\t%s\n", *alist);

			alist++;
		}

	if (tlist)
		while (*tlist)
			PRINTF ("\t\t%s\n", *tlist++);

	return;
}
