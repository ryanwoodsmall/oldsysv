/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/done.c	1.3"

#include "lpstat.h"

/**
 ** done() - CLEAN UP AND EXIT
 **/

void			done (rc)
	int			rc;
{
	(void)mclose ();

	if (!rc && exit_rc)
		exit (exit_rc);
	else
		exit (rc);
}
