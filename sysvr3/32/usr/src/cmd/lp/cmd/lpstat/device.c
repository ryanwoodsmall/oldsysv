/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/device.c	1.3"

#include "sys/types.h"

#include "lp.h"
#include "printers.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


static void		putdline();

/**
 ** do_device()
 **/

void			do_device (list)
	char			**list;
{
	register PRINTER	*pp;

	while (*list) {
		if (STREQU(NAME_ALL, *list))
			while ((pp = getprinter(NAME_ALL)))
				putdline (pp);

		else if ((pp = getprinter(*list)))
			putdline (pp);

		else {
			LP_ERRMSG1 (ERROR, E_LP_NOPRINTER, *list);
			exit_rc = 1;
		}

		list++;
	}
	return;
}

static void		putdline (pp)
	register PRINTER	*pp;
{
	if (!pp->device && !pp->dial_info) {
		LP_ERRMSG1 (ERROR, E_LP_PGONE, pp->name);
		done (1);

	} else if (pp->dial_info) {
		PRINTF ("dial token for %s: %s", pp->name, pp->dial_info);
		if (pp->device)
			PRINTF (" (on port %s)", pp->device);
		PRINTF ("\n");

	} else
		PRINTF ("device for %s: %s\n", pp->name, pp->device);

	return;
}
