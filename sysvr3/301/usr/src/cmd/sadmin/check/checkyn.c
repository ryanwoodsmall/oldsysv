/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/checkyn.c	1.4"
/*
	chkyn - Get Yes/No Response from User or Check Answer to Question
*/

#include <stdio.h>

#include "check.h"
#include <stddef.h>

extern char	*pgm_name;

/*	exit values */
#define	NO	1
#define	YES	0


main(argc, argv)
int	argc;
char	**argv;
{
	register char	**av;	/* stand-in for argv */
	char 	answer[MAXLINE];	/* array for input */
	register int	c;
	int	cycle = 0;	/* number of times question has been asked */
	int	cyclemax = CYCLEDEF;	/* max number of times to ask question */
	char	*defstr = 0;
	int	defval = 0;
	extern char	*optarg;	/* used by getopt() */
	extern int	optind;		/* used by getopt() */
	register int	force = 0;	/* force an answer */
	register char	*p;

	pgm_name = argv[0];
	errcode = argc + 1;
	setbuf( stdin, NULL );
	quitname = "quit";
	usagestr = "question ...\n";

	/* EXAMINE OPTIONS */
	while( (c = getopt( argc, argv, "atfeD:k:q:Q:h:H:m:!" )) != EOF ) {
		switch( c ) {
		case 'a':	/* permit non-printing and non-ascii answers */
			anychar = 1;
			break;
		case 't':	/* ignore white space at beginning and end of
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
			defstr = optarg;
			switch( *defstr ) {
			case 'y':
			case 'Y':
				defval = YES;
				break;
			case 'n':
			case 'N':
				defval = NO;
				break;
			default:
				error( "Illegal value for -D, '%s'.  Must begin with 'y' or 'n'.\n",
					defstr );
			}
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
	if( argc < 1 )
		usagemsg();
	helpchk();

	for( cycle = 0;  cycle < cyclemax;  cycle++ ) {
		for (av = argv; *av; av++)
			fprintf(stderr, "%s ", *av);
		fputs( "[y, n", stderr );
		if( helpstr )
			fprintf(stderr, ", %s", helpstr);
		if( quitstr )
			fprintf(stderr, ", %s", quitstr);
		fputs( "] ", stderr );
		fflush( stderr );
		if( !getans( answer ))
			continue;
		p = answer;
		if( !*p  &&  defstr )	/* no answer */
			gotit( defval );
	   nextchar:
		switch( *p ) {
		case ' ':
		case '\t':
			p++;
			goto nextchar;
		case 'y':
		case 'Y':
			if( force ) {
				switch( *++p ) {
				case '\0':
				case ' ':
					goto gotYES;
				case 'e':
				case 'E':
					switch( *++p ) {
					case 's':
					case 'S':
						switch( *++p ) {
						case '\0':
						case ' ':
							goto gotYES;
						}
					}
				}
				goto askagain;
			}
		   gotYES:
			gotit( YES );
			break;
		case 'n':
		case 'N':
			if( force ) {
				switch( *++p ) {
				case '\0':
				case ' ':
					goto gotNO;
				case 'o':
				case 'O':
					switch( *++p ) {
					case '\0':
					case ' ':
						goto gotNO;
					}
				}
				goto askagain;
			}
		   gotNO:
			gotit( NO );
			break;
		default:
			break;
		}
		if( !force )
			gotit( NO );
	    askagain:
		fputs( "\tType  y  for yes,  n  for no.", stderr );
		quithelp();
		if( defstr )
			fprintf( stderr, "\tdefault value  %c\n",
				defval ? 'n' : 'y' );
	}
	fprintf( stderr, "Too many attempts.  %s.", quitname );
	quit();
}


static
gotit( val )
int	val;
{
	if( echo ) {
		puts( val ? "n" : "y" );
	}
	exit( val );
}
