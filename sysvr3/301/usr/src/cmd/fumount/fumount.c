/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fumount:fumount.c	1.9"

#include <stdio.h>
#include <sys/types.h>
#include <sys/message.h>
#include <sys/idtab.h>
#include <sys/rfsys.h>
#include <sys/nserve.h>
#include <nserve.h>
#include "fumount.h"

#define MAXGRACE 3600

char *malloc(), *getnode();
char usage[] = "Usage: fumount [-w <seconds(0-%d)>] <resource>\n";

extern	int	optind,	opterr;
extern	char	*optarg;

struct clnts *client;

main(argc, argv)
int argc;
char **argv;
{
	char dom_resrc[(2 * MAXDNAME) + 2];
	char cmd[24+SZ_RES];
	char *clntlst[];
	char *grace = "0";
	int i, advflg = 0;

	while ((i = getopt(argc,argv,"w:")) != EOF)
		switch(i) {
 		case 'w':
			grace = optarg;
			if(((i = atoi(grace)) < 1) || (i > MAXGRACE)) {
				fprintf(stderr, usage, MAXGRACE);
				exit(1);
			}
			break;
		case '?':
			fprintf(stderr, usage, MAXGRACE);
			exit(1);
		}
	optarg = argv[optind];
	if(optarg[0] == '\0') {
		fprintf(stderr, usage, MAXGRACE);
		exit(1);
	}

	if(geteuid() != 0) {
		printf("Fumount: Must be Super User\n");
		exit(1);
	}
	if (rfsys(RF_GETDNAME, dom_resrc, MAXDNAME) < 0) {
		fprintf(stderr,"Can't get domain name. ");
		perror("fumount");
		exit(1);
	}
	
	strcat(dom_resrc, ".");
	strcat(dom_resrc, optarg);

	sprintf(cmd, "unadv %s >/dev/null 2>&1\n", optarg);
	if(system(cmd))
		advflg = NOTADV;
	if(nlload()){
		printf(stderr,"Fumount: can't get enough memory\n");
		exit(1);
	}
	if(getnodes(optarg, advflg) == 1) {
		if(advflg == NOTADV)
			fprintf(stderr,"fumount: %s not known\n",optarg);
	} else
			/* execute remote warning script */
		for(i = 0; client[i].flags != EMPTY; i++)
			if(client[i].flags & KNOWN)
				sndmes(client[i].sysid, grace, dom_resrc);

		/* sleep 'grace' seconds */
	sleep(atoi(grace));

		/* blow them away */
	if(rfsys(RF_FUMOUNT, optarg) == -1) {
		if(advflg & NOTADV) {
			printf("fumount: %s not known\n", optarg);
			exit(1);
		} else {
			if(client[0].flags != EMPTY) {	/* at least 1 client */
				perror("fumount: rfsys(2) failed.");
				exit(1);
			}
		/* failure of rfsys is not an error if no clients were 
		   using the resource */
		}
	}
	exit(0);
}
