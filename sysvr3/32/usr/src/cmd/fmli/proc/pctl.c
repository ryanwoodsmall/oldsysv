/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/pctl.c	1.1"

#include <stdio.h>
#include <varargs.h>
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"
#include "terror.h"
#include "ctl.h"

#define MAX_PROC_TITLE	40

extern struct proc_rec PR_all[];

int
proc_ctl(rec, cmd, arg1, arg2, arg3, arg4, arg5, arg6)
struct actrec *rec;
int cmd;
int arg1, arg2, arg3, arg4, arg5, arg6;
{
	static char title[MAX_PROC_TITLE];
	int p = rec->id;
	register int len, i;

	switch (cmd) {
	case CTGETITLE:
		if (rec->odptr) {
			**((char ***)(&arg1)) = rec->odptr;
		} else {
			len = sprintf(title, "%.*s ", MAX_PROC_TITLE-1, PR_all[p].name);
			i = 1;
			while (len<MAX_PROC_TITLE - 1 && i<MAX_ARGS && PR_all[p].argv[i]) {
				len += sprintf(title+len, "%.*s ", MAX_PROC_TITLE-1-len,
							filename(PR_all[p].argv[i]));
				i++;
			}
			**((char ***)(&arg1)) = &title[0];
		}
		return(SUCCESS);
	case CTGETPID:
		*((int *)arg1) = PR_all[rec->id].respid;
	case CTSETPID:
		PR_all[rec->id].respid = arg1;
		return(SUCCESS);
	default:
		return(FAIL);
	}
}
