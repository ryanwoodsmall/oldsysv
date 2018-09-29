/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/mkmenus.c	1.5.1.3"
/*
	descend a tree of directory looking for menus of commands
*/

#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<stdio.h>
#include	<string.h>

/* from SOLID */
#include	<cmderr.h>
#include	<stddef.h>

#define	MAXCNT	25
static char	*list[MAXCNT];
static char	*cmdlist[MAXCNT];
static char	cmddir[MAXCNT];
static int	cnt = 0;
static char	heading[ BUFSIZ/2 ];
static char	menulist[ 3*BUFSIZ ];
static char	*cmdargs[ 3*MAXCNT + 15 ];

static FILE	*lsdir();
static void	quit();


main( argc, argv )
int	argc;
char	*argv[];
{
	static int	ac;
	static char	buf[BUFSIZ];
	static int	cmdindex;
	static FILE	*fp;
	static int	i;
	static char	*menuname = 0;
	int		opt;
	extern char	*optarg;
	extern int	optind;
	static char	*pid;	/* character representation of process ID */
	static int	question;	/* index of "the question" in cmdargs[]
					*/
	static struct stat	stat;
	static char	shortquestion[80];
	static int	status;

	pgm_name = argv[0];
	while( (opt = getopt( argc, argv, "m:")) != EOF ) {
		switch( opt ) {
		case 'm':
			menuname = optarg;
			break;
		default:
			exit( 1 );
		}
	}
	while( optind-- )
		BUMPARG;

	if( argc < 1 ) {
		fprintf( stderr, "Usage:  %s directory [ pid ... ]\n",
			pgm_name );
		exit(1);
	}
	Chdir( argv[0] );
	if( !menuname )
		menuname = argv[0];

	while( (fp = lsdir( ".", buf )) != NULL ) {
		if( EQ( buf, "DESC" ) ) {
			descfile( fp, heading );
			continue;
		}
		Fstat( fileno( fp ), &stat );
		if( stat.st_mode & S_IFREG ) {
			normalfile( fp, buf );
		}
		else if( stat.st_mode & S_IFDIR ) {
			directory( fp, buf );
		}
		else
			cmderr( CERROR, "Illegal file mode '%#o', file '%s'",
				stat.st_mode, buf );
	}
	if( cnt <= 0 ) {
		printf( "\n\tNo subcommands are in menu '%s'.\n", menuname );
		exit( 0 );
	}
	ac = 0;
	cmdargs[ ac++ ] = "checklist";
	cmdargs[ ac++ ] = "-fp";
	if( argc > 1 ) {
		cmdargs[ ac++ ] = "-q^";
		cmdargs[ ac++ ] = "-Qgo back";
	}
	cmdargs[ ac++ ] = "-k";
	sprintf( buf, "%d", getpid() );
	cmdargs[ ac++ ] = pid = strdup( buf );
	/*	cmdargs[ question ] changes between the menulist
		and the short question.
	*/
	question = ac;
	strcpy( menulist, heading );
	for( i = 0;  i < cnt;  i++ ) {
		sprintf( buf, "%3d %-14s %s", i+1, cmdlist[i], list[i] );
		strcat( menulist, buf );
	}
	strcat( menulist, "\nEnter a number, a name, the initial part of a name, or\n? or <number>? for HELP,  " );
	if( argc > 1 )
		strcat( menulist, "^ to GO BACK,  " );
	strcat( menulist, "q to QUIT:" );
	cmdargs[ ac++ ] = menulist;
	/*	selection can be made by subcommand number or name.
	*/
	for( i = 1;  i <= cnt;  i++  ) {
		sprintf(buf, "%d", i );
		cmdargs[ ac++ ] = strdup( buf );
	}
	for( i = 0;  i < cnt;  i++  ) {
		cmdargs[ ac++ ] = cmdlist[i];
	}
	/*	help answers
	*/
	for( i = 1;  i <= cnt;  i++  ) {
		sprintf( buf, "%d?", i );
		cmdargs[ ac++ ] = strdup( buf );
	}
	cmdargs[ ac++ ] = "q";
	cmdargs[ ac++ ] = "q?";
	cmdargs[ ac++ ] = "^?";
	cmdargs[ ac++ ] = "?";
	cmdargs[ ac ] = 0;
	/*	when cmdargs[ ac ], above, is given a non-zero value,
		the following items become part of the argument list.
	*/
	cmdargs[ ac+1 ] = "<RETURN>";
	cmdargs[ ac+2 ] = 0;
	sprintf( shortquestion,
		"\nPress the RETURN key to see the %s menu [?, ", menuname );
	if( argc <= 1 ) {
		/* top menu */
		signal( SIGINT, quit );
		signal( SIGTERM, quit );
		strcat( shortquestion, "q]:" );
	}
	else
		strcat( shortquestion, "^, q]:" );
	while( 1 ) {
		cmdindex = ask( cmdargs ) - 1;
		if( cmdindex < cnt  ||  (cmdindex -= cnt) < cnt ) {
			if( cmddir[cmdindex] ) {
				if( nextdir( cmdlist[cmdindex], argc, argv,
					pid ) )
					goto fulllist;
			}
			else
				run( cmdlist[cmdindex] );
		}
		else if( (cmdindex -= cnt) < cnt ) {
			gethelp( cmdlist[ cmdindex ], cmddir[ cmdindex ] );
		}
		else switch( cmdindex -= cnt ) {
		case 0:	/* q */
			killexit( &argv[1] );
		case 1:	/* q? */
		case 2:	/* ^? */
			quithelp( argc );
			break;
		case 3:	/* ? */
			gethelp( "DESC", 0 );
			break;
		case 4:	/* "" */
		case 5:	/* "<RETURN>" */
	    fulllist:
			cmdargs[ question ] = menulist;
			cmdargs[ ac ] = 0;
			continue;
		default:
			cmderr( CHALT, "Unexpected cmdindex %d, cnt %d\n",
				cmdindex, cnt );
		}
		cmdargs[ question ] = shortquestion;
		cmdargs[ ac ] = "";
	}
}


static
normalfile( fp, f )
FILE	*fp;
char	*f;
{
	char	*a;
	char	buf[BUFSIZ];

	while( fgets( buf, sizeof buf, fp ) != NULL ) {
		if( (a = strpbrk( buf, " \t" )) == NULL )
			continue;
		*a = '\0';
		a++;
		if( EQ( buf, "#menu#" ) ){
			if( cnt >= MAXCNT )
				cmderr( CERROR, "Too many menu items, max = %d",
					MAXCNT );
			list[cnt] = strdup( a );
			cmdlist[cnt] = strdup( f );
			cnt++;
			return;
		}
	}
}


static
directory( fp, f )
FILE	*fp;
char	*f;
{
	char	buf[BUFSIZ];

	strcpy( buf, f );
	strcat( buf, "/DESC" );
	if( (fp = freopen( buf, "r", fp )) == NULL )
		return;
	*(strchr( buf, '/' )) = '\0';
	cmddir[cnt] = 1;
	normalfile( fp, buf );
}


static
descfile( fp, heading )
FILE	*fp;
char	*heading;
{
	char	*a;
	char	buf[BUFSIZ];

	strcpy( heading, "\n" );
	while( fgets( buf, sizeof buf, fp ) != NULL ) {
		if( (a = strpbrk( buf, " \t\n" )) == NULL )
			continue;
		if( EQN( buf, "#head#", 6 ) ){
			strcat( heading, a );
		}
	}
	strcat( heading, "\n" );
}


ask( av )
char	**av;
{
	int		exitcode;
	int		pid;
	int		status;

	switch( pid = fork() ) {
	case 0:
		Execvp( av[0], av );
	case -1:
		cmderr( CERROR, CERRNO );
	}
	waitpid( pid, &status );
	exitcode = status>>8;
	if( exitcode == 0 )
		exit( 1 );	/* checklist died due to signal */
	return  exitcode;
}


static
waitpid( pid, statusp )
int	pid;
int	*statusp;
{
	int	rc;

	while( (rc = wait( statusp )) != pid  &&  rc != -1 )
		;
	return  rc;
}


static
gethelp( f, isdir )
char	*f;
int	isdir;
{
	char	*a;
	char	buf[BUFSIZ];
	FILE	*fp;

	if( isdir ) {
		strcpy( buf, f );
		strcat( buf, "/DESC" );
		f = buf;
	}
	fp = Fopen( f, "r" );
	while( fgets( buf, sizeof buf, fp ) != NULL ) {
		if( (a = strpbrk( buf, " \t\n" )) == NULL )
			continue;
		if( EQN( buf, "#help#", 6 ) ){
			fputs( a, stderr );
		}
	}
	fclose( fp );
}


/*	Run this command again for the new directory and with the process ID
	of this process added to the argument list.
*/
static
nextdir( newdir, argc, argv, pid )
char	*newdir;
int	argc;
char	**argv;
char	*pid;
{
	static char	**av = 0;
	int		i;
	int		childpid;
	int		(*prevsig)();
	int		status;

	if( !av ) {
		av = (char**) Malloc( (argc+3) * sizeof( char* ) );
		av[0] = pgm_name;
		for( i = 0;  i < argc;  i++ )
			av[i+1] = argv[i];
		av[i+1] = pid;
		av[i+2] = 0;
	}
	prevsig = signal( SIGINT, SIG_IGN );
	if( !( childpid = fork() ) ) {
		av[1] = newdir;
		signal( SIGINT, prevsig );
		Execvp( av[0], av );
	}
	waitpid( childpid, &status );
	signal( SIGINT, prevsig );
	switch( status ) {
	case SIGINT:
	case SIGTERM:
		return  1;
	}
	return  0;
}


static
run( s )
char	*s;
{
	char	buf[BUFSIZ];

	sprintf( buf, "sh %s", s );
	system( buf );
}
		


static void
quit()
{
	exit( 0 );
}


static
quithelp( ac )
int	ac;
{
	fputs( "\tType  q  to quit the menu system entirely.\n", stderr );
	if( ac > 1 )
		fputs( "\tType  ^  to go back to the previous menu.\n",
			stderr );
	else
		fputs( "\tThe \"go back\",  ^  , function is not available on the first menu.\n",
			stderr );
}


/*	on successive calls to lsdir( directory ) return an open pointer
	to a file in directory and the file name (in fname).
	The files are opened in sorted order by name.
	Files names begining with "." are ignored.
*/
static
FILE*
lsdir( dir, fname )
char	*dir;
char	*fname;
{
	int	dncomp();
	static int	dircnt;		/* number of entries in directory */
	static struct direct	*direct = 0;
	static FILE	*fp = 0;

	/*	First time through, produce a sorted list of the
		directory entries.
	*/
	if( !direct ) {
		FILE		*dirp;
		struct stat	statbuf;

		dirp = Fopen( dir, "r" );
		Fstat( fileno( dirp ), &statbuf );
		direct = (struct direct *)Malloc( statbuf.st_size );
		dircnt = statbuf.st_size/sizeof *direct;
		Fread( direct, statbuf.st_size, 1, dirp );
		fclose( dirp );
		qsort( direct, dircnt, sizeof *direct, dncomp );
	}
	if( fp )
		fclose( fp );
	fp = 0;
	for( ;  dircnt--;  direct++ ) {
		if( direct->d_ino != 0  &&  direct->d_name[0] != '.' ) {
			strncpy( fname, direct->d_name, sizeof direct->d_name );
			if( (fp = fopen( fname , "r" )) != NULL ) {
				direct++;
				return  fp;
			}
		}
	}
	return  NULL;
}


/*	directory name compare
	return negative, 0, positive for d1->dname
	less than, equal, greater than d2->dname
*/
dncomp( d1, d2 )
struct direct	*d1, *d2;
{
	return  strncmp( d1->d_name, d2->d_name, sizeof d1->d_name );
}


static
killexit( av )
char	**av;
{
	signal( SIGTERM, SIG_IGN );
	for( ;  *av;  av++ )
		kill( atoi( *av ), SIGTERM );
	exit( 0 );
}
