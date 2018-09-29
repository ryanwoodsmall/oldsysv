/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	lock -- set a semaphore lock file for rmnttab
	unlock -- delete the lock file
*/

#ident	"@(#)rmount:mntlock.c	1.1.1.1"
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define RSEM_FILE "/usr/nserve/.rmnt.lock"

extern int errno;
extern char *cmd;

static int lfd = -1;

void
lock()
{
	while ((lfd = creat(RSEM_FILE, 0600)) < 0 && errno == EAGAIN)
		sleep(1);
	if (lfd < 0)
		fprintf(stderr, "%s: warning: cannot create temp file %s\n",
			cmd, RSEM_FILE);
	if (lockf(lfd, F_LOCK, 0L) < 0)
		fprintf(stderr, "%s: warning: cannot lock temp file %s\n",
			cmd, RSEM_FILE);
}

void
unlock()
{
	if (lfd >= 0)
		close(lfd);
}
