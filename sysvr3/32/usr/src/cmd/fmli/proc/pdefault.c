/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/pdefault.c	1.3"

#include <stdio.h>
#include "wish.h"
#include "terror.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"

struct proc_rec PR_all[MAX_PROCS];

/* make a default process, i.e. one that takes over the full screen */

proc_id
proc_default(flags, argv)
int flags;
char *argv[];
{
	register int i;
	int index;
	char *expand();

	if ((index = find_freeproc()) == FAIL) {
		mess_temp("Too many suspended activities!  Use frm-mgmt list to resume and close some.");
		return(FAIL);
	} 
	
#ifdef _DEBUG
	_debug(stderr, "Creating process at %d\n", index);
#endif
	PR_all[index].argv[0] = PR_all[index].name = expand(argv[0]);
#ifdef _DEBUG
	_debug(stderr, "PROCESS: %s", PR_all[index].name);
#endif
	for (i = 1; argv[i]; i++) {
		PR_all[index].argv[i] = expand(argv[i]);
#ifdef _DEBUG
		_debug(stderr, " %s", PR_all[index].argv[i]);
#endif
	}
#ifdef _DEBUG
	_debug(stderr, "\n");
#endif
	PR_all[index].argv[i] = NULL;
	PR_all[index].ar =  NULL;
	PR_all[index].status =  ST_RUNNING;
	PR_all[index].flags = flags;

	PR_all[index].pid = PR_all[index].respid = NOPID;
	return(index);
}

static int
find_freeproc()
{
	register int i;

	for (i = 0; i < MAX_PROCS; i++)
		if (PR_all[i].name == NULL)
			return(i);
	return(FAIL);
}
