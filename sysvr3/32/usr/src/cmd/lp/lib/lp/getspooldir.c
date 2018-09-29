/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/getspooldir.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "unistd.h"

#include "lp.h"

extern char		*getenv();

char			*getspooldir ()
{
	static char		*spooldir	= 0;


#if	defined(MALLOC_3X)
	/*
	 * WARNING: Don't use "malloc()" to store the name of
	 * the spooling directory; there may be occasion to reset
	 * the allocated space to reduce the process size.
	 */
#endif

	if (!spooldir) {
#if	defined(SPOOLDIR_FROM_ENV)
		spooldir = getenv("SPOOLDIR");
		if (
			!spooldir
		     || !*spooldir
		     || Access(spooldir, F_OK) == -1
		)
#endif
			spooldir = SPOOLDIR;
	}
	return (spooldir);
}
