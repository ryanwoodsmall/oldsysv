/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/checklist.c	1.5"
/*
	chklist - Get An Answer That Is One From A List
*/

#include <stdio.h>
#include <string.h>

#include "check.h"
#include <stddef.h>

extern char	*pgm_name;
static int	cklist();
static int	force = 0;		/* force an acceptable answer */
static char	*list = 0;		/* characters that separate the list
					items; doubles as option flag */
static int	pmatch = 0;
static char	stderrbuf[BUFSIZ];


main(argc, argv)
int	argc;
char	**argv;
{
	register char	**av;	/* stand-in for argv */
	char 	answer[MAXLINE];	/* array for input */
	register int	j, c;
	int	cycle = 0;	/* number of times question has been asked */
	int	cyclemax = CYCLEDEF;	/* max number of times to ask question */
	extern char	*optarg;	/* used by getopt() */
	extern int	optind;		/* used by getopt() */
	char	*question;

	pgm_name = argv[0];
	errcode = argc + 1;
	setbuf( stdin, NULL );
	setbuf( stderr, stderrbuf );
	quitname = "quit";
	usagestr = "question answer1 answer2 ...\n\t-l list-separators\t-p";

	/* EXAMINE OPTIONS */
	while( (c = getopt( argc, argv, "l:patfeD:k:q:Q:h:H:m:!" )) != EOF ) {
		switch( c ) {
		case 'l':	/* accept a list of answers */
			list = optarg;
			if( !*list )
				error( "-l must be followed by a string of separator characters.\n" );
			break;
		case 'p':	/* accept a partial match */
			pmatch = 1;
			break;
		case 'a':	/* permit non-printing and non-ascii answers */
			anychar = 1;
			break;
		case 't':	/* ignore leading and trailing white space in
				answer */
			trimspace = 1;
			break;
		case 'f':	/* force an answer */
			force = 1;
			break;
		case 'e':	/* echo answer */
			echo = 1;
			break;
		case 'D':	/* the default value for an empty input */
			defaultstr = optarg;
			break;
		case 'k':	/* quit sends  SIGTERM  to this process id */
			if( !num( optarg ) )
				error( "-k must be followed by a process ID number\n" );
			killpid = optarg;
			break;
		case 'q':	/* input that causes quit */
			quitstr = optarg;
			break;
		case 'Q':	/* alias name for quit */
			quitname = optarg;
			break;
		case 'h':	/* string to request help */
			helpstr = optarg;
			break;
		case 'H':	/* help message */
			helpmsg = optarg;
			break;
		case 'm':	/* maximum number of times to ask question */
			cyclemax = atoi( optarg );
			if( cyclemax <= 0 )
				error("bad value for -m '%s'\n", optarg);
			break;
		case '!':
			esc = 1;
			break;
		default:
			usagemsg();
		}
	}
	while( optind-- )
		BUMPARG;
	helpchk();
	if( list  &&  pmatch )
		error( "-l and -p flags cannot be used together.\n" );

	if( argc < 2 ) {
		usagemsg();
	}
	question = *argv;
	BUMPARG;
	for( cycle = 0;  cycle < cyclemax;  cycle++ ) {
		fprintf( stderr, "%s ", question );
		fflush( stderr );
		if( !getans( answer ))
			continue;
		if( list ) {
			if( j = rightlist( answer, argc, argv, list, cklist ) )
				exit( j );
		}
		else {
			if( j = cklist( answer, argv ) ) {
				exit ( j );
			}
		}
		if( !force )
			exit( errcode );
		fputs( "\tPermissible answers are", stderr );
		if( list )
			fputs( " (you may select more than one)", stderr );
		else if( pmatch )
			fputs( " (a partial match will select one)", stderr );
		fputs( ":\n\t", stderr );
		j = 9;
		for( av = argv; *av; av++ ) {
			c = strlen( *av ) + 1;
			if( j + c > LINELEN ) {
				fputs( "\n\t", stderr );
				j = 9;
			}
			j += c;
			fprintf( stderr,"%s ", *av );
		}
		fputs( "\n", stderr );
		quithelp();
	}
	fprintf( stderr, "Too many attempts.  %s.", quitname );
	quit();
}


/*
	Look for a right answer in list.  If found, return its number
	(1 thru N), else return 0.
*/
static
cklist( a, argv )
register char	*a;
register char	**argv;
{
	char		**av;
	register int	j;
	int		l;
	register int	m = 0;

	av = argv;
	for( j = 1;  *argv;  argv++, j++ ) {
		if( EQ( a, *argv ) ) {
			if( echo  &&  !list )
				puts( a );
			return  j;
		}
	}
	if( pmatch  &&  (l = strlen( a )) ) {
		/* Accept a partial match provided there is only one. */
		argv = av;
		for( j = 1;  *argv;  argv++, j++ ) {
			if( EQN( a, *argv, l ) ) {
				if( !m )
					m = j;
				else {
					if( force )
						fputs( "\tAnswer partially matches several possibilities.\n\tIt must match only one to be accepted.\n", stderr );
					return  0;
				}
			}
		}
		if( m  &&  !list ) {
			if( echo )
				puts( av[ m-1 ] );
			return  m;
		}
	}
	return  0;
}
