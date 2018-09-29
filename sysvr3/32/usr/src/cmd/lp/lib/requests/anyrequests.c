/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/requests/anyrequests.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "lp.h"
#include "fs.h"

/**
 ** anyrequests() - SEE IF ANY REQUESTS ARE ``QUEUED''
 **/

int			anyrequests ()
{
	long			lastdir		= -1;

	if (!Lp_Requests) {
		getpaths ();
		if (!Lp_Requests)
			return (1);	/* err on safe side */
	}

	if (!(next_file(Lp_Requests, &lastdir)))
		return (0);
	else
		return (1);
}
