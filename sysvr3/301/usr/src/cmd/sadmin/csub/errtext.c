/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/errtext.c	1.2"
/*	
	Routines to print and adjust options on error messages.
	Command and library version.
*/

#include	"cmderr.h"
#include	<stdio.h>
#include	<string.h>
#include	"switch.h"

int	errexit = 1;
static	char	*emsgs[] = {
		"",
		"WARNING:  ",
		"ERROR:  ",
		"HALT:  ",
		0
	};
char	**errmessages = emsgs;

static	char	*advice = 0;
static	int	bell = 0;
static	int	tag = 0;
static	int	tagnum = 0;
static	char	*tagstr = 0;
static	int	text = 1;
static	char	*tofix = 0;

char	*getenv();


void
errtext( severity, format, ErrArgList )
int	severity;
char	*format;
int	ErrArgList;
{
	errbefore( severity, format, ErrArgList );
	errverb( getenv( "ERRVERB" ) );
	if( bell )
		fputs( "\07", stderr );
	if( text ) {
		char	*s;

		if( s = getenv( "ERRPREFIX" ) )
			fputs( s, stderr );
		fputs( errmessages[ severity ], stderr );
	}
	if( (text  ||  tag)  &&  pgm_name )
		pgmname();
	if( text ) {
		if( (int)format == CERRNO )
			perror("");
		else {
			fprintf( stderr, format, ErrArgList );
			fputs( "\n", stderr );
		}
	}
	if( tag ) {
		if( tagstr )
			fprintf( stderr, "\t%s", tagstr );
		if( tagnum )
			fprintf( stderr, "%5d", tagnum );
		fputs( "\n", stderr );
	}
	if( (text  ||  tag)  &&  advice )
		fprintf( stderr, "\t%s\n", advice );
	if( (text  ||  tag)  &&  tofix )
		fprintf( stderr, "To Fix:\t%s\n", tofix );
	errafter( severity, format, ErrArgList );
	return;
}

void
erradvice( str )
char	*str;
{
	advice = str;
}

void
errtag( str, num )
char	*str;
int	num;
{
	tagstr = str;
	tagnum = num;
}

void
errtofix( str )
char	*str;
{
	tofix = str;
}


void
errverb( s )
register char	*s;
{
	char	buf[ BUFSIZ ];
	register
	 char	*token;
	static
	 char	space[] = ", \t\n";

	if( !s )
		return;
	strcpy( buf, s );
	token = strtok( buf, space );
	do {
		SWITCH( token )
		CASE( "bell" )
			bell = 1;
		CASE( "nobell" )
			bell = 0;
		CASE( "tag" )
			tag = 1;
		CASE( "notag" )
			tag = 0;
		CASE( "text" )
			text = 1;
		CASE( "notext" )
			text = 0;
		ENDSW
	} while( token = strtok( (char*)0, space ) );
}
