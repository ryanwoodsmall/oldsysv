/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:nsquery.c	1.2"
#include  <stdio.h>
#include  "nserve.h"

extern	int	errno;
extern	int	optind,	opterr;
extern	char	*optarg;

main(argc,argv)
int	argc;
char	*argv[];
{


	int	chr;
	int	errflg;
	int	hflg;
	char	*name = "*";
	char 	*usage = "nsquery [-h] [<name>]";


	errflg = hflg = 0;
	if (argc != 1) {
		while ((chr = getopt(argc,argv,"h")) != EOF)
			switch(chr) {
	 		case 'h':
				hflg++;	
				break;
			case '?':
				errflg++;
				break;
			}
	}
	if (argv[optind] != NULL)
		name = argv[optind];
	
	if (errflg) {
		fprintf(stderr,"%s\n",usage);
		exit(1);
	}

	if (!hflg)
		fprintf(stdout,"   Resource               Description                Server\n");

	if (ns_info(name) == FAILURE) {
		fprintf(stderr,"nsquery: error\n");
		exit(1);
	}
}
