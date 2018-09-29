/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.proc:src/proc/suspend.c	1.3"

#include <stdio.h>
#include <signal.h>
#include "wish.h"

int
suspend(cmd)
char *cmd;
{
	char suspath[40];
	int vpid;
	FILE *fp;
	void sig_nothing();
	
	signal(SIGUSR1, sig_nothing);

	if ((vpid = atoi(getenv("VPID"))) == 0) {
#ifdef _DEBUG
		_debug(stderr, "Unable to get VPID\n");
#endif
		return(FAIL);
	}

	sprintf(suspath, "/tmp/suspend%d", vpid);
	if ((fp = fopen(suspath, "w")) == NULL) {
#ifdef _DEBUG
		_debug(stderr, "Unable to open suspend file %s\n", suspath);
#endif
		return(FAIL);
	}
	(void) fprintf(fp, "%d\n%s\n", getpid(), cmd ? cmd : "");
	(void) fclose(fp);

	if (kill(vpid, SIGUSR1) == FAIL) {
#ifdef _DEBUG
		_debug(stderr, "Unable to send sigusr1 to face pid=%d\n", vpid);
#endif
		return(FAIL);
	}
	pause();
	return(SUCCESS);
}


static void
sig_nothing(sig)
int sig;
{
	/* do nothing, just catch the signal and return */
	return;
}
