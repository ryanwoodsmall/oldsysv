/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idload:idload.c	1.7"
#include <stdio.h>
#include <sys/types.h>
#include <sys/idtab.h>
#include <sys/rfsys.h>

extern	char *optarg;
extern	int   optind;

main(argc, argv)
int   argc;
char *argv[];
{
	int	n_update = 0;
	int	error = 0;
	int	num_args = 0;
	int	gflag = 0, uflag = 0;
	int	usr_map_stat = 0, grp_map_stat = 0;
	int	c;

	char	*g_rules = NULL;
	char	*u_rules = NULL;
	char	*passdir = NULL;

	/*
	 *	Parse arguments.
	 */

	while ((c = getopt(argc, argv, "ng:u:")) != EOF) {
		switch (c) {
			case 'n':
				n_update = 1;
				break;
			case 'g':
				if (gflag)
					error = 1;
				else {
					g_rules = optarg;
					gflag = 1;
				}
				break;
			case 'u':
				if (uflag)
					error = 1;
				else {
					u_rules = optarg;
					uflag = 1;
				}
				break;
			case '?':
				error = 1;
		}
	}

	num_args = argc - optind;

	if (num_args == 1)
		passdir = argv[optind];

	if (num_args > 1 || error) {
		fprintf(stderr, "%s: usage: %s [-n] [-g gid_rules] [-u uid_rules] [directory]\n", argv[0], argv[0]);
		exit(1);
	}

	/*
	 *	Clear the translation tables and
	 *	perform user id mapping for all machines.
	 */

	if (n_update == 0)
		rfsys(RF_SETIDMAP, NULL, UID_MAP, NULL);

	usr_map_stat = uidmap(UID_MAP, u_rules, passdir, NULL, n_update);

	/*
	 *	Perform group id mapping for all machines.
	 */

	grp_map_stat = uidmap(GID_MAP, g_rules, passdir, NULL, n_update);

	if (grp_map_stat == 0 || usr_map_stat == 0)
		exit(1);
	else
		exit(0);
}
