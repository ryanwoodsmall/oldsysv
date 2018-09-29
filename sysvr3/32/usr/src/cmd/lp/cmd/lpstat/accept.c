/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/accept.c	1.3"

#include "stdio.h"

#include "lp.h"
#include "class.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"

/**
 ** do_accept()
 **/

void			do_accept (list)
	char			**list;
{
	while (*list) {
		if (STREQU(*list, NAME_ALL)) {
			send_message (S_INQUIRE_CLASS, "");
			(void)output (R_INQUIRE_CLASS);
			send_message (S_INQUIRE_PRINTER_STATUS, "");
			(void)output (R_INQUIRE_PRINTER_STATUS);

		} else if (isclass(*list)) {
			send_message (S_INQUIRE_CLASS, *list);
			(void)output (R_INQUIRE_CLASS);

		} else {
			send_message (S_INQUIRE_PRINTER_STATUS, *list);
			switch (output(R_INQUIRE_PRINTER_STATUS)) {
			case MNODEST:
				LP_ERRMSG1 (ERROR, E_LP_BADDEST, *list);
				exit_rc = 1;
				break;
			}

		}
		list++;
	}
	return;
}

/**
 ** putqline()
 **/

void			putqline (
				dest,
				rejecting,
				reject_date,
				reject_reason
			)
	char			*dest;
	int			rejecting;
	char			*reject_date,
				*reject_reason;
{
	PRINTF ("%s ", dest);
	if (!rejecting)
		PRINTF ("accepting requests since %.24s\n", reject_date);
	else
		PRINTF (
			"not accepting requests since %.24s -\n\t%s\n",
			reject_date,
			reject_reason
		);
	return;
}
