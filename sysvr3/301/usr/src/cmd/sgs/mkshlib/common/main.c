/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/main.c	1.4"

#include <stdio.h>
#include <signal.h>
#include "paths.h"
#include "filehdr.h"
#include "ldfcn.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"

main(argc, argv)
int	argc;
char	**argv;
{
	progname= argv[0];
	prefix= getpref(progname);
	ldname= concat(prefix,"ld");
	asname= concat(prefix,"as");

	cmdline(argc, argv);	/* process the command line */
	mktmps();		/* get names of all temporary files */
	catchsigs();		/* catch signals to prevent leaving temp files */
	rdspec();		/* read shared lib. specification file */

	if (maketarg == TRUE)	/* make the target */
		mktarget();

	if (makehost == TRUE)	/* make the host */
		mkhost();
	rmtmps();		/* remove all the temporary files */
	return(0);
}

/* This routine processes the command line using getopt(3) */
void
cmdline(namec, namev)
int 	namec;
char	**namev;
{
	extern char	*optarg;
	extern int	optind;
	int	c;
	int	sflag=FALSE,
		tflag=FALSE,
		nflag=FALSE;

	/* process command line and set specfnam */
	while ((c = getopt(namec, namev, "s:t:h:nq")) != EOF)
		switch (c) {
			case 's':
				sflag=TRUE;
				specname= optarg;
				break;
			case 't':
				tflag=TRUE;
				trgname= optarg;
				break;
			case 'h':
				makehost= TRUE;
				hstname= optarg;
				break;
			case 'n':
				nflag=TRUE;
				break;
			case 'q':
				qflag=TRUE;
				break;
			case '?':
			default:
				usagerr();
		}

	if (!sflag) {
		(void)fprintf(stderr,"The -s option must be supplied.\n");
		usagerr();
	}

	if (!tflag) {
		(void)fprintf(stderr,"The -t option must be supplied.\n");
		usagerr();
	}

	if (optind < namec)
		usagerr();

	if (!nflag)
		maketarg= TRUE;
}

void
usagerr()
{
	(void)fprintf(stderr,"usage: %smkshlib -s specfil -t target [-h host] [-n]\n", prefix);
	exit(1);
}
