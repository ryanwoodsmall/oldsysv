/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	rumount -- dequeue a mount request

	usage: umount resource ...

	exit codes:
		0: 1 or more resources were removed from the queue
		1: no resources were removed from the queue
		2: syntax or functional error
*/

#ident	"@(#)rmount:rumount.c	1.1.1.1"
#include <sys/types.h>
#include <nserve.h>
#include <mnttab.h>
#include <stdio.h>

char *cmd;

main(argc, argv)
int argc;
char **argv;
{
	int chg = 0;			/* flag if rmnttab is altered */
	register int i;			/* argv index */
	int rc = 0;			/* return code */
	char *fqn(), fdev[MAXDNAME], fres[MAXDNAME];	/* fully qualified names */
	struct mnttab *rd_rmnttab();
	struct mnttab *mtab, *last_entry;
	register struct mnttab *mp;	/* table pointer */

	cmd = argv[0];			/* set command name for diagnostics */

	if (geteuid() != 0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		return 2;
	}

	if (argc < 2) {
		fprintf(stderr, "usage: %s resource ...\n", cmd);
		return 2;
	}

	lock();
	if (mtab = rd_rmnttab(&last_entry)) {	/* read rmnttab */

		/* run through the table, looking for the entries */
		for (mp = mtab; mp < last_entry; ++mp) {
			fqn(mp->mt_dev, fdev);
			for (i = 1; i < argc; ++i)
				if (*argv[i] && !strcmp(fdev, fqn(argv[i], fres))) {
					/* mark for deletion */
					*mp->mt_dev = '\0';
					*argv[i] = '\0';
					++chg;
				}
		}

		/* some resources may be not queued. If so, list them out */
		if (chg != argc - 1) {
			fprintf(stderr, "%s: resources not queued:", cmd);
			for (i=1; i < argc; ++i)
				if (*argv[i])
					fprintf(stderr, " %s", argv[i]);
			fputc('\n', stderr);
		}

		/* if all resources are not queued, don't rewrite the file */
		if (!chg) {
			unlock();
			return 1;
		}

		/* write rmnttab */

		if (!wr_rmnttab(mtab, last_entry))
			rc = 2;		/* error in write */ 
	}
	else
		rc = 2;		/* error in reading rmnttab */
	unlock();
	return rc;
}
