/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/checkchild.c	1.1"

#include	"lpsched.h"

/**
 ** ev_checkchild() - CHECK FOR DECEASED CHILDREN
 **/

void			ev_checkchild ()
{
	register EXEC		*ep	= &Exec_Table[0],
				*epend	= &Exec_Table[ET_Size];


	/*
	 * This routine is necessary to find out about child
	 * processes that disappear without a trace. An example
	 * of how they might disappear: kill -9 pid.
	 * To minimize a race condition with a dying child,
	 * we don't mark the child as gone unless it hasn't been
	 * seen for two cycles.
	 */
	for ( ; ep < epend; ep++)
		if (ep->pid > 0 && kill(ep->pid, 0) == -1)
			if (ep->flags & EXF_GONE) {
				ep->pid = -99;
				ep->status = SIGTERM;
				ep->errno = 0;
				DoneChildren++;
			} else
				ep->flags |= EXF_GONE;

	schedule (EV_LATER, WHEN_CHECKCHILD, EV_CHECKCHILD);
	return;
}
