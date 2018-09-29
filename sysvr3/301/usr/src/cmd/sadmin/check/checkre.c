/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/checkre.c	1.4"
/*
	chkre - Check an answer against a series of Regular Expressions
*/

#include <stdio.h>

#include "check.h"
#include <cmderr.h>
#include <stddef.h>
#include <switch.h>

extern char	*pgm_name;

static enum mode mode = REGEX;

static int	force = 0;		/* if force, insist on a match */
static int	ckregex();


main(argc, argv)
register int	argc;
register char	**argv;
{
	char 	answer[MAXLINE];	/* array for input */
	register int	c;
	int	cycle = 0;	/* number of times question has been asked */
	int	cyclemax = CYCLEDEF;	/* max number of times to ask question */
	char	*list = 0;		/* characters that separate the list
					items; doubles as option flag */
	extern char	*optarg;	/* used by getopt() */
	extern int	optind;		/* used by getopt() */
	char	*question;

	pgm_name = argv[0];
	errcode = argc + 1;
	setbuf( stdin, NULL );
	quitname = "quit";
	usagestr = "-[r|R] question regex1 mesg1 [ [ -r|R ] re2 m2 ]...";

	/* EXAMINE OPTIONS */
	while( (c = getopt( argc, argv, "l:atferRD:k:q:Q:h:H:m:!" )) != EOF ) {
		switch( c ) {
		case 'l':	/* accept a list of answers */
			list = optarg;
			break;
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
		case 'r':	/* accept regular expression(s) that match */
			mode = REGEX;
			break;
		case 'R':	/* reject regular expression(s) that match */
			mode = REGEXR;
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
	if( argc < 3 )
		usagemsg();
	helpchk();

	question = *argv;
	BUMPARG;
	for( ;  argc >= 2 ;  BUMPARG, BUMPARG ) {
		char	*prevmessage = *argv;

		SWITCH( *argv )
		CASE( "-r" )
			mode = REGEX;
			BUMPARG;
		CASE( "-R" )
			mode = REGEXR;
			BUMPARG;
		ENDSW
		if( *argv[1] == '\0' )
			argv[1] = prevmessage;
		compregex( argv[0], argv[1], mode );
		prevmessage = argv[1];
	}
	if( argc > 0 ) {
		usagemsg();
	}
	for( cycle = 0;  cycle < cyclemax;  cycle++ ) {
		fprintf(stderr, "%s ", question);
		fflush( stderr );
		if( !getans( answer ) )
			continue;
		if( list ) {
			if( rightlist( answer, argc, argv, list, ckregex ) )
				exit( 0 );
		}
		else {
			if( ckregex( answer ) ) {
				if( echo )
					puts( answer );
				exit ( 0 );
			}
		}
		if( !force )
			exit( errcode );
		quithelp();
	}
	fprintf( stderr, "Too many attempts.  %s.", quitname );
	quit();
}


/*
	handle regular expressions in the list
*/
#define INIT	register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return
#undef	ERROR		/* to avoid collision with stddef.h */
#define ERROR(c)	error( reerrformat, regerror(c) )
static char		reerrformat[] = "RE error, %s\n";
#include  "regexp.h"

#define	EXPSIZE	512
struct cregex {		/* compiled regular expression */
	char	cstr[EXPSIZE];	/* compiled RE string */
	int	circf;		/* circumflex (beginning anchor) flag */
	int	reject;		/* 1 == message is printed if RE matches */
	char	*msg;		/* message printed when string is rejected */
	struct cregex	*next;	/* next regular expression */
};

static struct cregex	*start = 0;	/* points to first cregex */

/*
	compile and store the regular expressions
*/
static
compregex( regexp, msg, mode )
char	*regexp;
char	*msg;
enum mode mode;
{
	extern int	circf;	/* circumflex flag set by compile() */
	char		*malloc();
	static struct cregex	*oldp;	/* points to previous cregex */
	register struct cregex	*p;
	
	p = (struct cregex*)malloc( sizeof( struct cregex ) );
	if( !start )
		start = p;
	else
		oldp->next = p;
	oldp = p;
	compile( regexp, p->cstr, &(p->cstr[EXPSIZE]), '\0' );
	p->circf = circf;
	p->reject = ( mode == REGEXR );
	p->msg = msg;
	p->next = 0;
	return;
}

/*
	check the input against the regular expressions
*/
static
ckregex( input )
char	*input;
{
	register struct cregex	*p;
	extern int	circf;	/* used by step() */
	register int	or;	/* 1 if successive REs are or'ed and only
				one has to "pass" */

	/*	check each cregex in the chain */
	for( p = start;  p;  p = p->next ) {
		circf = p->circf;
		or = EQ( p->msg, "-o" );
		if( !( step( input, p->cstr ) ^ p->reject ) ) {
			if( or )
				continue;
			if( force )
				fprintf( stderr, "\t%s\n", p->msg );
			return  0;
		}
		if( or ) {
			for( p = p->next;  p;  p = p->next ) {
				if( !EQ( p->msg, "-o" ) ) {
					break;
				}
			}
			if( !p )
				cmderr( CERROR, "Last message field was '-o'" );
		}
	}
	return  1;
}
