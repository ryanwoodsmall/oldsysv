/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/cancel.c	1.2"

#include "lpsched.h"

/**
 ** cancel() - CANCEL A REQUEST
 **/

int			cancel (prs, spool)
	register RSTATUS	*prs;
	int			spool;
{
	if (prs->request->outcome & RS_DONE)
		return (0);

	prs->request->outcome |= RS_CANCELLED;

	if (spool)
		prs->request->outcome |= RS_NOTIFY;

	if (prs->request->outcome & RS_PRINTING)
		terminate (prs->printer->exec);
	else if (prs->request->outcome & RS_FILTERING)
		terminate (prs->exec);
	else if (spool)
		notify (prs, (char *)0, 0, 0, 0);
	else
		check_request (prs);

	return (1);
}
