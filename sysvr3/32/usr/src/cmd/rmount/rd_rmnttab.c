/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	rd_mnttab - read the rmount table
		a block is allocated that is large enough to hold
		the table plus room for one more entry.
	return:
		pointer to the start of the rmount table,
		NULL if error
		parameter last points to the entry past the last one
*/

#ident	"@(#)rmount:rd_rmnttab.c	1.1.1.1"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <mnttab.h>
#include <stdio.h>
#include <fcntl.h>

#define RMNTTAB	"/usr/nserve/rmnttab"
#define MSIZE	sizeof(struct mnttab)

extern char *cmd;

struct mnttab *
rd_rmnttab(last)
struct mnttab **last;
{
	char *malloc();
	int mfd, mtab_size;
	struct mnttab *mtab;
	struct stat stbuf;

	if (stat(RMNTTAB, &stbuf) < 0) {
		if (errno == ENOENT)	 	/* rmnttab does not exist */
			stbuf.st_size = 0;	/* pretend it's empty */
		else {
			fprintf(stderr, "%s: cannot stat %s\n", cmd, RMNTTAB);
			return(NULL);
		}
	}
	else if ((mfd = open(RMNTTAB, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: cannot open %s\n", cmd, RMNTTAB);
		return(NULL);
	}

	mtab_size = stbuf.st_size + sizeof(struct mnttab);

	if (!(mtab = (struct mnttab *)malloc(mtab_size))) {
		fprintf(stderr, "%s: cannot allocate space for %s\n",
			cmd, RMNTTAB);
		return(NULL);
	}

	if (stbuf.st_size && (read(mfd, mtab, stbuf.st_size) != stbuf.st_size)) {
		fprintf(stderr, "%s: read error on %s\n", cmd, RMNTTAB);
		return(NULL);
	}

	close(mfd);
	*last = (struct mnttab *)((char *)mtab + stbuf.st_size);
	return(mtab);
}
