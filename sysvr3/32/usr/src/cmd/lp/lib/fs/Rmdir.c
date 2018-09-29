/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/Rmdir.c	1.2"

#include "fslib.h"

/**
 ** Rmdir() - SIMULATE/INVOKE SVR3 "rmdir()" SYSTEM CALL
 **/

int			Rmdir (path)
	char			*path;
{
	int			rc;

	switch (have_SVR3) {

	case -1:
		see_if_SVR3 ();
		return (Rmdir(path));

	case 0:
		return (s5rmdir(path));

	case 1:
		while ((rc = rmdir(path)) == -1 && errno == EINTR)
			;
		return (rc);

	}
	/*NOTREACHED*/
}
