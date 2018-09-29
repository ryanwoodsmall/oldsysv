/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/Mkdir.c	1.2"

#include "fslib.h"

/**
 ** Mkdir() - SIMULATE/INVOKE SVR3 "mkdir()" SYSTEM CALL
 **/

int			Mkdir (path, mode)
	char			*path;
	int			mode;
{
	int			rc;

	switch (have_SVR3) {

	case -1:
		see_if_SVR3 ();
		return (Mkdir(path, mode));

	case 0:
		return (s5mkdir(path, mode));

	case 1:
		while ((rc = mkdir(path, mode)) == -1 && errno == EINTR)
			;
		return (rc);
	}
	/*NOTREACHED*/
}

