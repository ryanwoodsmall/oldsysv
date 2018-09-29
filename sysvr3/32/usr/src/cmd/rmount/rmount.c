/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	rmount: queue a resource for mount

	usage:	rmount -d[-r] resource directory

*/

#ident	"@(#)rmount:rmount.c	1.1.2.1"
#include <sys/types.h>
#include <nserve.h>
#include <mnttab.h>
#include <stdio.h>

#define RMNTTAB	"/usr/nserve/rmnttab"	/* rmount table */

char *cmd;

main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	int dflg=0, rflg = 0;		/* command line flags */
	static char usage[] = "usage: %s [-d[r] resource directory]\n";

	cmd = argv[0];
	if (argc == 1) {		/* dump contents of rmnttab */
		return rshow();
	}
	else if (geteuid() != 0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		return 1;
	}
	while ((c = getopt(argc, argv, "dr")) != -1)
		switch (c) {
		case 'd':
			if (dflg)
				err = 1;
			else
				dflg = 1;
			break;
		case 'r':
			if (rflg)
				err = 1;
			else
				rflg = 1;
			break;
		case '?':
			err = 1;
			break;
		}
	if (err || !dflg || optind+2 != argc) {
		fprintf(stderr, usage, cmd);
		return 1;
	}
	else
		return renter(argv[optind], argv[optind+1], (dflg << 1)|rflg);
}


/*
	renter - enter a request onto rmnttab
*/

extern char *cmd;

renter(resource, dir, flag)
char *resource, *dir;
short flag;
{
	char *fqn();	/* convert a name into a fully qualified domain.name */
	struct mnttab *mstart, *mp, *last, *rd_rmnttab();
	char fres[MAXDNAME], fdev[MAXDNAME];	/* fully qualified names */
	int err = 0;		/* return value */

	lock();

	fqn(resource, fres);	/* generate a fully qualified name */
	/* look for conflicts between rmnttab entries and the new one */

	if ((mstart = rd_rmnttab(&last))) {
		for(mp = mstart; mp < last; mp++) {
			if (strcmp(fqn(mp->mt_dev, fdev), fres) == 0) {
				unlock();
				printf("%s: %s already queued for mount\n",
					cmd, resource);
				return 1;
			}
		}
		strncpy(mp->mt_dev, resource, 32);
		strncpy(mp->mt_filsys, dir, 32);
		mp->mt_ro_flg = flag;
		mp->mt_time = time((long *)0);

		/* write out the rmnttab file */
		if (!wr_rmnttab(mstart, last+1))
			err = 1;
	}
	else		/* problems reading rmnttab */
		err = 2;
	unlock();
	return err;
}


/*
	rshow - show pending mounts (dump rmnttab)
*/

rshow()
{
	char *cftime();
	struct mnttab *mp, *last, *rd_rmnttab();
	char ptime[40];
	static char *flg[] = { "   ", "-r ", "-d ", "-dr" };

	if ((mp = rd_rmnttab(&last))) {
		for(; mp < last; mp++) {
			cftime(ptime, (char *)0, &mp->mt_time);
			printf("%-16.32s %-20.32s %s %s\n",
				mp->mt_dev, mp->mt_filsys,
				flg[(mp->mt_ro_flg) & 0x3], ptime);
		}
		return 0;
	}
	else
		return 1;
}
