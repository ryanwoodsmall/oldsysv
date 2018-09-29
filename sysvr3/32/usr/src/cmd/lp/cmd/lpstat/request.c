/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/request.c	1.5"

#include "stdio.h"
#include "pwd.h"
#include "sys/types.h"

#include "lp.h"
#include "msgs.h"
#include "requests.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


/**
 ** do_request()
 **/

void			do_request (list)
	char			**list;
{
	while (*list) {
		if (STREQU(NAME_ALL, *list)) {
			send_message (S_INQUIRE_REQUEST, "", "", "", "", "");
			(void)output (R_INQUIRE_REQUEST);

		} else if (isrequest(*list)) {
			send_message (S_INQUIRE_REQUEST, "", "", *list, "", "");
			switch (output(R_INQUIRE_REQUEST)) {
			case MNOINFO:
				LP_ERRMSG1 (ERROR, E_STAT_DONE, *list);
				exit_rc = 1;
				break;
			}

		} else {
			send_message (S_INQUIRE_REQUEST, "", *list, "", "", "");
			switch (output(R_INQUIRE_REQUEST)) {
			case MNOINFO:
				if (!isprinter(*list) && !isclass(*list)) {
					LP_ERRMSG1 (ERROR, E_STAT_BADSTAT, *list);
					exit_rc = 1;
				}
				break;
			}

		}
		list++;
	}
	return;
}

/**
 ** do_user()
 **/

void			do_user (list)
	char			**list;
{
	while (*list) {
		if (STREQU(NAME_ALL, *list)) {
			send_message (S_INQUIRE_REQUEST, "", "", "", "", "");
			(void)output (R_INQUIRE_REQUEST);

		} else {
			send_message (S_INQUIRE_REQUEST, "", "", "", *list, "");
			switch (output(R_INQUIRE_REQUEST)) {
			case MNOINFO:
				if (!getpwnam(*list))
					LP_ERRMSG1 (WARNING, E_STAT_USER, *list);
				break;
			}
		}
		list++;
	}
	return;
}

/**
 ** putoline()
 **/

void			putoline (
				request_id,
				user,
				size,
				date,
				state,
				printer,
				form,
				character_set
			)
	char			*request_id,
				*user,
				*date,
				*printer,
				*form,
				*character_set;
	long			size;
	int			state;
{
	PRINTF (
		"%-*s %-*s %*ld   %.12s",
		IDSIZE,
		request_id,
		LOGMAX-1,
		user,
		OSIZE,
		size,
		date + 4
	);


	if (!verbose) {

		/*
		 * Unless the -l option is given, we show the CURRENT
		 * status. Check the status bits in reverse order of
		 * chronology, i.e. go with the bit that would have been
		 * set last. Old bits don't get cleared by the Spooler.
		 * We only have space for 21 characters!
		 */

		if (state & RS_NOTIFYING)
			PRINTF (" notifying user");

		else if (state & RS_CANCELLED)
			PRINTF (" canceled");

		else if (state & RS_PRINTED)
			PRINTF (" finished printing");

		else if (state & RS_PRINTING)
			PRINTF (" on %s", printer);

		else if (state & RS_ADMINHELD)
			PRINTF (" held by admin");

		else if (state & RS_HELD)
			PRINTF (" being held");

		else if (state & RS_FILTERED)
			PRINTF (" filtered");

		else if (state & RS_FILTERING)
			PRINTF (" being filtered");

		else if (state & RS_CHANGING)
			PRINTF (" held for change");

	} else {
		register char		*sep	= "\n	";

		/*
		 * Here we show all the interesting states the job
		 * has gone through. Left to right they are in
		 * chronological order.
		 */

		if (state & RS_PRINTING) {
			PRINTF ("%son %s", sep, printer);
			sep = ", ";

		} else if (state & RS_CANCELLED) {
			PRINTF ("%scanceled", sep);
			sep = ", ";

		} else if (state & RS_FAILED) {
			PRINTF ("%sfailed", sep);
			sep = ", ";

		} else if (state & RS_PRINTED) {
			PRINTF ("%sfinished on %s", sep, printer);
			sep = ", ";

		/*
		 * WATCH IT! We make the request ID unusable after
		 * the next line.
		 */
		} else if (!STREQU(strtok(request_id, "-"), printer)) {
			PRINTF ("%sassigned %s", sep, printer);
			sep = ", ";

		} else {
			PRINTF ("%squeued for %s", sep, printer);
			sep = ", ";
		}

		if (!(state & RS_DONE)) {
			if (form && *form) {
				PRINTF ("%sform %s", sep, form);
				sep = ", ";
			}
			if (character_set && *character_set) {
				PRINTF ("%scharset %s", sep, character_set);
				sep = ", ";
			}
		}

		if (state & RS_NOTIFYING) {
			PRINTF ("%snotifying user", sep);
			sep = ", ";

		} else if (state & RS_CHANGING) {
			PRINTF ("%sheld for change", sep);
			sep = ", ";

		} else if (state & RS_ADMINHELD) {
			PRINTF ("%sheld by admin", sep);
			sep = ", ";

		} else if (state & RS_HELD) {
			PRINTF ("%sbeing held", sep);
			sep = ", ";

		}

		if (state & RS_FILTERED) {
			PRINTF ("%sfiltered", sep);
			sep = ", ";

		} else if (state & RS_FILTERING) {
			PRINTF ("%sbeing filtered", sep);
			sep = ", ";
		}

	}
	PRINTF ("\n");
	return;
}
