/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/Readdir.c	1.2"

#include "fslib.h"

/**
 ** Readdir() - SIMULATE/INVOKE SVR3 "readdir()" SYSTEM CALL
 **/

struct dirent		*Readdir (dirp)
	register DIR		*dirp;
{
	struct dirent		*rc;

	switch (have_SVR3) {

	case -1:
		see_if_SVR3 ();
		return (Readdir(dirp));

	case 0:
		return (s5readdir(dirp));

	case 1:
		/*
		 * From the looks of the SVR3.1 code for readdir(),
		 * this looks OK for re-entering readdir() after
		 * an interrupt, except if the read being attempted
		 * would have resulted in EOF. The impact is that
		 * telldir() may lie. We don't need telldir() on EOF,
		 * so we should be OK.
		 */
		while (!(rc = readdir(dirp)) && dirp->dd_size == -1 && errno == EINTR)
			dirp->dd_size = 0;
		return (rc);

	}
	/*NOTREACHED*/
}
