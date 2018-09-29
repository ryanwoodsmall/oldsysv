/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/list.c	1.1"

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
#include "menudefs.h"

extern struct proc_rec PR_all[];


struct actrec *
menline_to_proc(n)
int n;
{
	register int i, l;

	for (i = l = 0; i < MAX_PROCS; i++) {
		if (PR_all[i].name)
			if (++l == n)
				return(PR_all[i].ar);
	}
	return(NULL);
}

struct menu_line
proc_menudisp(n, ptr)
int n;
char *ptr;
{
	struct menu_line m;
	struct actrec *a;

	m.highlight = m.description = NULL;
	m.flags = 0;
	if ((a = menline_to_proc(n)) != NULL)
		ar_ctl(a, CTGETITLE, &m.highlight);	
	return(m);
}
