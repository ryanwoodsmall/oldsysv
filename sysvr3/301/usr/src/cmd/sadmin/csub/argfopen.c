/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:csub/argfopen.c	1.2"
/*	
	Returns FILE pointers to successfully opened files picked off the
	argc/argv list.
*/

#include	<stdio.h>
#include	<stddef.h>
#include	<switch.h>

extern	int	argf_dir;	/* if non-zero, change directories when possible
				to cut down overhead of opening files.
				Set if "-D[ optional directory name ]" appears
				as part of argument list.
				If negative, -D is not permitted.  "+D" as part
				of argument list sets argf_dir negative. */

static	FILE	*filep;
static	int	first_call = 1;
static	char	tmpbuf[BUFSIZ];

#define	BUMP	(*argcp)--, (*argvp)++


FILE *
argfopen(argcp, argvp, type, error_action)
register int	*argcp;
register char	**argvp[];
register char	*type;
register int	error_action;
{
	FILE	*standard();
	FILE	*manfiles();

	if( first_call ) {
		first_call = 0;
		if( *argcp == 0  ||  *argvp == 0 )
			return  standard( type );
		if( EQ( **argvp, "+D" ) )
			argf_dir = -1;
	}
	for( ;  *argcp  &&  *argvp;  BUMP ) {
		file_name = **argvp;
		SWITCH( file_name )
		CASE( "-" )
			BUMP;
			return  standard( type );
		CASE( "--" )
			while(scanf("%s", tmpbuf) != EOF) {
				file_name = tmpbuf;
				if(filep = manfiles(file_name, type, 
						error_action))
					return  filep;
			}
		CASE( "+D" )
			if( argf_dir > 0 )
				complain( "-D already in effect, cannot change.",
					FATAL );
			argf_dir = -1;
		DEFAULT
			if(filep = manfiles(file_name, type, error_action)) {
				BUMP;
				return  filep;
			}

		ENDSW
	}
	return  NULL;
}


/*
	(manipulate files.) close old file. open new one if possible. if
	not possible, take appropriate action.
*/
static
FILE *
manfiles(fname, type, error_action)
char	*fname;
char	*type;
int	error_action;
{
	static char	dir_name[BUFSIZ];
	static FILE	*fp;
	static	int	file_opened = 0;
		char	*strrchr();

	if (file_opened) {
		fclose(fp);
		file_opened = 0;
	}
	if( !strncmp( fname, "-D", 2 ) ) {
		if( argf_dir < 0 )
			complain( "+D already in effect, cannot change.",
				FATAL );
		argf_dir = 1;
		fname +=2;
		if( *fname ) {
			strcpy( dir_name, fname );
			if( chdir( fname ) ) {
				complain( "Cannot change directories.",
					error_action );
				*dir_name = '\0';
			}
		}
		return  NULL;
	}
	if( argf_dir > 0  &&  *fname == '/' ) {
		char	*sp;

		sp = strrchr( fname+1, '/' );
		if( sp ) {
			*sp = '\0';
			if( !EQ( dir_name, fname ) ) {
				strcpy( dir_name, fname );
				if( chdir( dir_name ) ) {
					complain( "Cannot change directories.",
						error_action );
					*sp = '/';
					*dir_name = '\0';
					return  NULL;
				}
			}
			*sp = '/';
			fname = sp + 1;
		}
	}
	if ((fp = fopen(fname, type)) == NULL) {
		complain( "Cannot open file.", error_action );
		return  NULL;
	}
	file_opened = 1;
	return  fp;
}


/*
	decide on whether no arg or "-" means standard input or output based
	on type of fopen requested.
*/
static
FILE *
standard( type )
char	*type;
{
	static	char	std_in[] = "std in";
	static	char	std_out[] = "std out";

	switch( *type ) {
	case 'r':
		file_name = std_in;
		return  stdin;
	case 'a':
	case 'w':
		file_name = std_out;
		return  stdout;
	}
	fprintf( stderr, "argfopen():  illegal fopen() type \"%s\"\n",
		type );
	complain( (char *) 0, FATAL );
}


static
complain( str, error_action )
char	*str;
int	error_action;
{
	if( error_action == IGNORE )
		return;
	pgmname();
	filename();
	if( str )
		fprintf( stderr, "%s", str );
	switch( error_action ) {
	case IGNORE:
	case ERROR:
		break;
	case FATAL:
		fprintf( stderr, "  Program stopped.\n" );
		exit( FATAL );
	default:
		fprintf( stderr,
			"\nargfopen():  bad error_action  %d\n",
			error_action );
		abort();
	}
	fprintf( stderr, "\n" );
}
