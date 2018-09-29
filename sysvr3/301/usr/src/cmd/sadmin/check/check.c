/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:check/check.c	2.3"
/*
	common functions for the check routines
*/

#define	extern
#include "check.h"
#undef	extern

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>


/*
	print the error message and exit
*/
/*VARARGS1*/
error( format, a1, a2, a3, a4, a5 )
char	*format;
int	a1, a2, a3, a4, a5;
{
	pgmname();
	fprintf( stderr, format, a1, a2, a3, a4, a5 );
	exit( errcode );
}


/*
	get an answer from the standard input
*/
int
getans( a )
register char	*a;
{
	register char	*a2;

	if( gets(a) == NULL ) {
		fputc( '\n', stderr );
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
			sesystem( &helpmsg[1] );
		else
			fprintf(stderr, "%s\n\n", helpmsg);
		return  0;
	}
	if( defaultstr  &&  EQ(a, "") ) {
		if( echo )
			puts( defaultstr );
		exit(0);
	}
	/*	Normally, any string with non-printing characters other than
		space is rejected.
	*/
	if( !anychar ) {
		for( a2 = a;  *a2;  a2++ ) {
			if( !isprint(*a2) ) {
				fputs( "Input contains a non-printing character.  Try again.\n",
					stderr );
				return  0;
			}
		}
	}
	/*	When trimspace, leading and trailing white space is ignored.
	*/
	if( trimspace ) {
		for( a2 = a;  isspace(*a2);  a2++ );
		/*	If we found leading blanks, shift the string left.
			Leave a pointing to the '\0' at the end of the string.
		*/
		if( a2 != a ) {
			do {
				*(a++) = *a2;
			} while( *(a2++) );
			a--;
		}
		else
			while( *a )
				a++;
		/*	Count off the trailing blanks.  Replace the first
			trailing blank with a '\0'.
		*/
		a--;
		while( isspace(*a) )
			a--;
		a++;
		*a = '\0';
	}
	return  1;
}


/*
	check for proper help arguments
*/
helpchk()
{
	if( !helpstr  &&  helpmsg )
		helpstr = "?";
	else if( helpstr  &&  !helpmsg )
		error( "-H<message> must be specified with -h<string>.\n" );
}


/*
	kill the given process and then exit

	fflush() before kill() to improve chances of '\n' getting to terminal
	before output from killed process.
*/
quit()
{
	fputc( '\n', stderr );
	fflush( stderr );
	if( killpid )
		kill( atoi( killpid ), SIGTERM );
	exit( errcode );
}


/*
	print the quit, help, and default information, if present.
*/
quithelp()
{
	if( quitstr  ||  helpstr  ||  ( defaultstr  &&  *defaultstr ) )
		fputc( '\t', stderr );
	if( helpstr )
		fprintf(stderr, "%s  for help.  ", helpstr);
	if( quitstr )
		fprintf(stderr, "%s  to %s.  ", quitstr, quitname);
	if( ( defaultstr  &&  *defaultstr ) )
		fprintf(stderr, "default value  %s", defaultstr );
	fputc( '\n', stderr );
}


/*
	Run the "!cmd", if permitted.
*/
shellescape( answer )
char	*answer;
{
	if( esc )
		sesystem( &answer[1] );
	else
		fputs( "\t! escape not allowed\n", stderr );
}


/*
	print out standard usage message and exit
*/
usagemsg()
{
	fprintf( stderr, "Usage:  %s [ flags ] %s\n", pgm_name, usagestr );
	fputs( "\t-a	-e	-f	-t	-!\n", stderr );
	fputs( "\t-h helpstring  -H helpmessage\n", stderr );
	fputs( "\t-q quitstring  -Q quitname  -k killpid\n", stderr );
	fputs( "\t-D default-answer\n", stderr );
	exit( errcode );
}
