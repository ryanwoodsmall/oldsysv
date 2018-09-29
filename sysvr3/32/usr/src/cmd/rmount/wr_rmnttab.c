/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	wr_mnttab - write the mount table
		write successive struct mnttab entries from "rp"
		up to (not including) "last" where the mt_dev field
		is a non-empty string.
	return:
		1 on success, 0 on failure of write
*/

#ident	"@(#)rmount:wr_rmnttab.c	1.1.1.1"
#include <sys/types.h>
#include <signal.h>
#include <mnttab.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define RMNTTAB	"/usr/nserve/rmnttab"
#define MSIZE	sizeof(struct mnttab)

extern char *cmd;

wr_rmnttab(rstart, last)
struct mnttab *rstart, *last;
{
	struct mnttab *rp, *fillp;
	long fsize;
	int rfd, ret = 1;

	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	
	for (fillp = rp = rstart; rp < last; ++rp)
		if (*rp->mt_dev) {		/* non-empty slot */
			if (rp != fillp)	/* pack the table */
				memcpy((char *)fillp, (char *)rp, MSIZE);
			++fillp;
		}
	fsize = (char *)fillp - (char *)rstart;

	if ((rfd = creat(RMNTTAB, 0664)) < 0) {
		fprintf(stderr, "%s: cannot create %s\n", cmd, RMNTTAB);
		return 0;
	}

	if (fsize) {
		lockf(rfd, F_LOCK, 0L);		/* no reads during update */

		if (write(rfd, rstart, fsize) != fsize) {
			fprintf(stderr, "%s: write error on %s\n", cmd, RMNTTAB);
			ret = 0;		/* set return code */
		}
	}
	close(rfd);

	signal(SIGHUP,  SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT,  SIG_DFL);
	
	return ret;
}
