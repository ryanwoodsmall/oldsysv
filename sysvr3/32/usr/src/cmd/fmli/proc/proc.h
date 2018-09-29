/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/proc.h	1.1"

#define MAX_PROCS	5
#define MAX_ARGS	10

#define NOPID	-1
#define ST_RUNNING	0
#define ST_DEAD		1
#define ST_SUSPENDED	2

struct proc_rec {
	char *name;
	char *argv[MAX_ARGS+2];
	int status;			/* running, dead, or suspended */
	int flags;			/* prompt at end */
	int pid;			/* actual id of the process */
	int respid;			/* process id to resume */
	struct actrec *ar;	/* activation record proc is in */
};
