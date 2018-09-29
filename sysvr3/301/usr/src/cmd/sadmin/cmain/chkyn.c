/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/chkyn.c	2.1.1.1"
/*
	chkyn - Get Yes/No Response from User or Check Answer to Question
*/

#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <switch.h>

extern char	*pgm_name;

#define	FORCE	-1
#define	LINELEN	74
#define	MAXLINE	256
#define	NO 	1
#define	YES	0

static int	echo = 0;	/* flag for printing of answer */
static char	*defaultstr = 0;	/* default value returned for empty input */
static int	errcode;	/* error exit code */
static int	esc = 0;	/* flag to allow escape to shell */
static char	*helpstr = 0;	/* this string gives help */
static char	*helpmsg = 0;	/* this string is help message */
static int	killpid = 0;	/* kill this process ID upon quit */
static char	*quitname = "quit";	/* what the quit string is called */
static char	*quitstr = 0;	/* this string causes a quit */

enum	mode { YESNO, LIST, REGEX, REGEXR, ARITH } mode = YESNO;


main(argc, argv)
int argc;
char **argv;
{
	char	**av;	/* stand-in for argv */
	char 	answer[MAXLINE];	/* array for input */
	int	j, c;
	int	cycle = 0;	/* number of times question has been asked */
	int	cyclemax = 25;	/* max number of times to as question */
	FILE	*fpin, *fpout;	/* used in ARITH */
	int	num(), numd();	/* functions to check for numbers */
	int	(*numfp)();	/* pointer to num() or numd() */
	extern char	*optarg;	/* used by getopt() */
	extern int	optind;		/* used by getopt() */
	char	*question;
	int	x = 0;		/* flag for YES, NO, or FORCE */
	int	ynf = 0;	/* counter for -y, -n, -f options */

	pgm_name = argv[0];
	errcode = argc + 1;
	setbuf( stdin, NULL );

	/* EXAMINE OPTIONS */
	while( (c = getopt( argc, argv, "ynfecrRidD:k:q:Q:h:H:m:!" )) != EOF ) {
		switch( c ) {
		case 'y':	/* default yes */
			ynf++;
			x = YES;
			break;
		case 'n':	/* default no */
			ynf++;
			x = NO;
			break;
		case 'f':	/* force an answer */
			ynf++;
			x = FORCE;
			break;
		case 'e':	/* echo answer */
			echo = 1;
			break;
		case 'c':	/* check a list */
			mode = LIST;
			break;
		case 'r':	/* accept regular expression(s) that match */
			mode = REGEX;
			break;
		case 'R':	/* reject regular expression(s) that match */
			mode = REGEXR;
			break;
		case 'i':	/* match an integer expression */
			mode = ARITH;
			numfp = num;
			break;
		case 'd':	/* match an decimal expression */
			mode = ARITH;
			numfp = numd;
			break;
		case 'D':	/* the default value for an empty input */
			defaultstr = optarg;
			break;
		case 'k':	/* quit sends  SIGTERM  to this process id */
			if( !num( optarg ) )
				error( "-k must be followed by a process ID number\n" );
			killpid = atoi( optarg );
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
	if( argc < 1 ) {
		usagemsg();
	}
	if (ynf > 1) {
		error( "Only one of -y, -n, -f permitted.\n" );
	}
	if( ( helpstr  ||  helpmsg )  &&  !( helpstr  &&  helpmsg ) )
		error( "-h<string> and -H<message> must be specified together.\n" );

	switch( mode ) {
	case YESNO:
		if( defaultstr )
			error( "-D must be accompanied by -c, -r, -i, or -d.\n" );
		for( cycle;  cycle < cyclemax;  cycle++ ) {
			for (av = argv; *av; av++)
				fprintf(stderr, "%s ", *av);
			fputs( "[y, n", stderr );
			if( helpstr )
				fprintf(stderr, ", %s", helpstr);
			if( quitstr )
				fprintf(stderr, ", %s", quitstr);
			fputs( "] ", stderr );
			if( !getans( answer ))
				continue;
			switch (*answer) {
			case 'y':
			case 'Y':
				if( x == FORCE ) {
					switch (answer[1]) {
					case '\0':
					case ' ':
						goto gotYES;
					case 'e':
					case 'E':
						switch (answer[2]) {
						case 's':
						case 'S':
							switch (answer[3]) {
							case '\0':
							case ' ':
								goto gotYES;
							}
						}
					}
					goto askagain;
				}
			   gotYES:
				if( echo ) {
					printf( "%c\n", *answer );
				}
				exit( YES );
			case 'n':
			case 'N':
				if( x == FORCE ) {
					switch (answer[1]) {
					case '\0':
					case ' ':
						goto gotNO;
					case 'o':
					case 'O':
						switch (answer[2]) {
						case '\0':
						case ' ':
							goto gotNO;
						}
					}
					goto askagain;
				}
			   gotNO:
				if( echo ) {
					printf( "%c\n", *answer );
				}
				exit( NO );
			default:
				break;
			}
			if (x != FORCE)
				exit( x );
		    askagain:
			fputs( "\tType  y  for yes,  n  for no.", stderr );
			quithelp();
		}
		break;
	case LIST:
		if( argc < 2 ) {
			usagemsg();
		}
		question = *argv;
		BUMPARG;
		for( cycle;  cycle < cyclemax;  cycle++ ) {
			fprintf(stderr, "%s ", question);
			if( !getans( answer ))
				continue;
			for (j = 0; argv[j]; j++) {
				if (EQ(argv[j], answer)) {
					if( echo )
						printf("%s\n",argv[j]);
					exit (j + 1);
				}
			}
			if (x != FORCE)
				exit (errcode);
			fputs( "\tPermissible answers are:\n\t", stderr );
			j = 9;
			for (av = argv; *av; av++) {
				c = strlen( *av ) + 1;
				if( j + c > LINELEN ) {
					fputs( "\n\t", stderr );
					j = 9;
				}
				j += c;
				fprintf(stderr,"%s ", *av);
			}
			fputs( "\n", stderr );
			quithelp();
		}
		break;
	case REGEX:
	case REGEXR:
		if( argc < 3 )
			usagemsg();
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
		for( cycle;  cycle < cyclemax;  cycle++ ) {
			fprintf(stderr, "%s ", question);
			if( !getans( answer ))
				continue;
			if( ckregex( answer, x ) ) {
				if( echo )
					printf("%s\n", answer);
				exit(0);
			}
			if( x == FORCE ) {
				continue;
			}
			exit (errcode);
		}
		break;
	case ARITH:
		if( argc < 2 )
			usagemsg();
		question = *argv;
		BUMPARG;
		p2open( "bs", &fpin, &fpout );
		fprintf( fpin,
			"compile\nfun f(x)\nreturn(1 & ( %s ))\nnuf\nexecute\n",
			*argv );
		BUMPARG;
		for( cycle;  cycle < cyclemax;  cycle++ ) {
			char	s[50];

			fprintf(stderr, "%s ", question);
			if( !getans( answer ))
				continue;
			switch( *answer ) {
			case '-':
				if( !(*numfp)( &answer[1] ) )
					goto arithfail;
				break;
			default:
				if( !(*numfp)( answer ) )
					goto arithfail;
			}
			fprintf( fpin, "f(%s)\n", answer );
			fflush(fpin);
			fgets( s, sizeof s, fpout );
			if( EQ(s, "1\n") ) {
				if( echo )
					printf("%s\n", answer);
				exit(0);
			}
		    arithfail:
			if( x != FORCE ) {
				exit (errcode);
			}
			fputs( "\t", stderr );
			for (av = argv; *av; av++)
				fprintf(stderr,"%s ", *av);
			fputs( "\n", stderr );
			quithelp();
		}
		break;
	default:
		error( "bad mode, %d\n", mode );
	}
	fprintf( stderr, "Too many attempts.  %s.", quitname );
	quit();
}


/*
	handle regular expressions in the list
*/
#undef	ERROR	/* to avoid collision with stddef.h */
#define INIT	register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return
#define ERROR(c)	regerror(c)
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
	We reuse the argument pointer to point at the compiled RE.
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
	struct cregex	*p;
	
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
ckregex( input, x )
char	*input;
{
	register
	struct cregex	*p;
	extern int	circf;	/* used by step() */

	/*	check each cregex in the chain */
	for( p = start;  p;  p = p->next ) {
		circf = p->circf;
		if( !( step( input, p->cstr ) ^ p->reject ) ) {
			if( x == FORCE )
				fprintf( stderr, "\t%s\n", p->msg );
			quithelp();
			return  0;
		}
	}
	return  1;
}

static
regerror(code)
int	code;
{
	char	*msg;

	switch( code ) {
	case 11:
		msg = "Range endpoint too large";
		break;
	case 16:
		msg = "Bad number";
		break;
	case 25:
		msg = "'\\digit' out of range";
		break;
	case 36:
		msg = "Illegal or missing delimiter";
		break;
	case 41:
		msg = "No remembered search string";
		break;
	case 42:
		msg = "'\\( \\)' imbalance";
		break;
	case 43:
		msg = "Too many '\\('";
		break;
	case 44:
		msg = "More than 2 numbers given in \\{ \\}";
		break;
	case 45:
		msg = "} expected after \\";
		break;
	case 46:
		msg = "First number exceeds second in \\{ \\}";
		break;
	case 49:
		msg = "[] imbalance";
		break;
	case 50:
		msg = "regular expression too long or too complex";
		break;
	default:
		msg = "unknown code!";
	}
	error( "RE error, %s.\n", msg );
}


static
shellescape( answer )
char	*answer;
{
	if( esc )
		system (&answer[1]);
	else
		fputs( "\t! escape not allowed\n", stderr );
}


static char	*usage[] = {	/* usage message */
		"[ -ynf ] prompt ...\n",
		"-c question answer ...\n",
		"-r question regex1 mesg1 [ re2 m2 ]...\n",
		"-[id] question expression mesg ...\n",
		0
		};
static
usagemsg()
{
	char	**p;

	p = usage;
	fprintf( stderr, "Usage:  %s [ flags ] %s", pgm_name, *p );
	while( *(++p) )
		fprintf( stderr, "\t%s [ flags ] %s", pgm_name, *p );
	fputs( "flags ==  -!   -e   -q quitstring  -Q quitname  -k killpid\n", stderr );
	fputs( "\t  -h helpstring  -H helpmessage\n", stderr );
	exit(errcode);
}


static
error( format, a1, a2, a3, a4, a5 )
char	*format;
int	a1, a2, a3, a4, a5;
{
	pgmname();
	fprintf( stderr, format, a1, a2, a3, a4, a5 );
	exit(errcode);
}


static
getans( a )
char	*a;
{
	fflush( stderr );
	if( gets(a) == NULL ) {
		fputs( "\n", stderr );
		return  0;
	}
	if( *a == '!' ) {
		shellescape( a );
		return  0;
	}
	if( quitstr  &&  EQ(a, quitstr) )
		quit();
	if( helpstr  &&  EQ(a, helpstr) ) {
		if( *helpmsg == '!' )
			system( &helpmsg[1] );
		else
			fprintf(stderr, "%s\n\n", helpmsg);
		return  0;
	}
	if( defaultstr  &&  EQ(a, "") ) {
		if( echo )
			printf( "%s\n", defaultstr );
		exit(0);
	}
	return  1;
}


static
quit()
{
	if( killpid )
		kill( killpid, SIGTERM );
	fputs( "\n", stderr );
	exit(errcode);
}


static
quithelp()
{
	if( quitstr  ||  helpstr  ||  ( defaultstr  &&  *defaultstr ) )
		fputs( "\t", stderr );
	if( helpstr )
		fprintf(stderr, "%s  for help.  ", helpstr);
	if( quitstr )
		fprintf(stderr, "%s  to %s.  ", quitstr, quitname);
	if( ( defaultstr  &&  *defaultstr ) )
		fprintf(stderr, "default value  %s", defaultstr );
	fputs( "\n", stderr );
}


/*
	system(3S) with standard out redirected to standard error
	to avoid shell escapes from being taken as the answer.
*/
static
system(s)
char	*s;
{
	extern int fork(), execl(), wait();
	int	status, pid, w;
	register int (*istat)(), (*qstat)();

	if((pid = fork()) == 0) {
		close(1);	/* redirect standard out to standard err */
		dup(2);
		(void) execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != -1)
		;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return((w == -1)? w: status);
}
