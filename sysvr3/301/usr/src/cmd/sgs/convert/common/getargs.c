/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)convert:common/getargs.c	1.7"

#include	<stdio.h>
#include	"sgsmacros.h"
#include	"filehdr.h"
#include	"conv.h"

unsigned short	target,
		origin;
unsigned short	ar_target = CUR;  /* default target format is the format of
				     the current archive */
int		preswab;

extern char	*filev[],
		*malloc();

int
getargs(argc,argv)

register int	argc;
register char	**argv;
{
	char	fbuf[BUFSIZ];
	int	filec;
	char	*machine;

	filec = 0;

	while (--argc > 0) {
		++argv;
		if (argv[0][0] == '-') switch (argv[0][1]) {
			case '\0':
				while ( fscanf(stdin, "%s", fbuf) != EOF
						&& filec < MAXFILES ) {
					filev[filec] = malloc(strlen(fbuf)+1);
					if ( filev[filec] == NULL ) {
						converr(1,"***command line***",
								"out of space");
						return(-1);
						}
					strcpy(filev[filec++], fbuf);
					}

				if ( filec == MAXFILES ) {
					converr(1,"***command line***",
							"too many files");
					return(-1);
					}
				break;
			case 's':
				/* sgsconv -s replaces swab
				 * this means that the user
				 * intends to swab-dump onto
				 * the target machine so only
				 * preswab chars here
				 */
				preswab++;
				break;

			case 't':
			if ( target != 0) {
				converr(1,"***command line***",
					"only one target flag allowed");
				return(-1);
				}
			++argv;
			--argc;
			machine = *argv;
	
			if (IS32W(*argv)||IS16W(*argv)) {
				target = F_AR32W;
			} else {
				if ( IS32WR(*argv) ) {
					target = F_AR32WR;
				} else {
					if ( IS16WR(*argv) ) {
						target = F_AR16WR;
					} else {
						converr(1,"***command line***",
						  "unknown target %s", *argv);
						return(-1);
						}
					}
				}
				break;

			case 'p':
				/* output file should be in portable (5.0) archive format */
				ar_target = RANDOM;
				break;

			case 'o':
				/* output file should be in old (pre 5.0) archive format */
				ar_target = OLD;
				break;

			case 'a':
				/*
				 * output file should be in portable (6.0 ascii
				 * headers) archive format
				 */
				ar_target = ASCIIAR;
				break;

			default:
				converr(1,"***command line***",
					"unknown flag (%c)",argv[0][1]);
				return(-1);
				break;
		} else {
			if ( filec == MAXFILES ) {
				converr(1,"***command line***","too many files");
				return(-1);
				}
			filev[filec++] = *argv;
			}
	} /* while */

	if (target == 0)
		return(-1);

	if ((ar_target != OLD) && ( equal( machine, "pdp", 3 ) || equal( machine, "ibm", 3 ) || equal( machine, "i80", 3 ))) {
		converr(0, "***command line***", "portable archive format not supported on %s", machine );
		ar_target = OLD;
	}

	return(filec);
} /* getargc */
