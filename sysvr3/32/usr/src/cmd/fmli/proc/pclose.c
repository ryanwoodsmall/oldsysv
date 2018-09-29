/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/pclose.c	1.3"

#include <stdio.h>
#include <signal.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"
#include "terror.h"

extern struct proc_rec PR_all[];
extern int Vflag;

int
proc_close(rec)
register struct actrec	*rec;
{
	int	i;
	int	id;
	int	pid;
	int	oldsuspend;

	if (Vflag)
		showmail(TRUE);
	id = rec->id;
	pid = PR_all[id].pid;
#ifdef _DEBUG
	_debug(stderr, "closing process table %d, pid=%d\n", id, pid);
#endif
	if (pid != NOPID) {	/* force the user to close by resuming it */
#ifdef _DEBUG
		_debug(stderr, "FORCING CLOSE ON PID %d\n", pid);
#endif
		oldsuspend = suspset(FALSE);	/* disallow suspend */
		PR_all[id].flags |= PR_CLOSING;
		ar_current(rec);
		suspset(oldsuspend);
	}
	for (i = 0; i < MAX_ARGS && PR_all[id].argv[i]; i++)
		free(PR_all[id].argv[i]);
	PR_all[id].name = NULL;
	PR_all[id].status = ST_DEAD;
	if (rec->path)
		free(rec->path);
	if (rec->odptr)
		free(rec->odptr);
	return SUCCESS;
}
