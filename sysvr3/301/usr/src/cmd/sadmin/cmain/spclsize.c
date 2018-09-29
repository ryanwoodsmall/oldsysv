/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sadmin:cmain/spclsize.c	1.3.1.2"
/*	
	Determine the current size of a file by lseek(2)ing and reading
	around in it until you get to the last byte.  This is the only
	known way to get the size of a block- or character-special file.
*/

#include	<cmderr.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<values.h>


static int	debug = 0;


main( argc, argv )
int	argc;
char	*argv[];
{
	int	c;
	int	errflg = 0;
	char	*filename = 0;
	extern
	 int	optind;
	long	scale = 1;

	pgm_name = *argv;
	while( ( c = getopt( argc, argv, "bfx" ) ) != EOF ) {
		switch( c ) {
		case 'b':	/* report in blocks */
			scale = 512;
			break;
		case 'f':	/* preceed value with file name */
			filename = *argv;
			break;
		case 'x':	/* unadvertised debug option */
			debug = 1;
			break;
		default:
			errflg = 1;
		}
	}
	while( optind-- ) {
		argc--;
		argv++;
	}
	if( errflg  ||  argc < 1 ) {
		fprintf( stderr, "Usage:  %s [ -b ] [ -f ] file ...\n", pgm_name );
		exit( 1 );
	}

	for( ;  *argv;  argv++ ) {
		if( filename )
			filename = *argv;
		process( *argv, filename, scale );
	}
	exit( 0 );
}


static
process( file, filename, scale )
char	*file, *filename;
int	scale;
{
	long	current = MAXLONG;
	int	diff;
	int	direction;
	int	fd;
	long	largest, smallest;

	fd = Open( file, O_RDONLY | O_NDELAY );

	if( (largest = lseek( fd, 0L, 2 )) > 0 ) {
		report( filename, largest, scale );
		return;
	}
	else if( largest == -1 ) {
		cmderr( CWARN, "Unable to determine size of file %s.", file );
		return;
	}

	smallest = 0;
	largest = current;
	direction = -1;
		
	while( 1 ) {
		if( debug )
			printf( " %ld\n", current );
		switch( lookat( file, fd, current ) ) {
		case 0:
			report( filename, current, scale );
			return;
		case 1:
			direction = 1;
			smallest = current;
			break;
		case -1:
			direction = -1;
			largest = current;
			break;
		default:
			abort();
		}
		diff = (largest - smallest)/2;
		current += direction * ( diff ? diff : 1 );
	}
}


static
lookat( filename, fd, spot )
char	*filename;
int	fd;
long	spot;
{
	char	buf;
	static
	 long	lastspot = -1;
	long	lseek();

	if( lastspot == spot )
		cmderr( CERROR, "lookat() is stuck, spot repeated %ld", spot );
	lastspot = spot;
	if( spot == 0 )
		return  0;
	if( lseek( fd, spot, 0 ) == -1 )
		cmderr( CHALT, "lseek() failure, spot = %ld", spot );
	return  read( fd, &buf, sizeof buf );
}


static
report( filename, size, scale )
char	*filename;
long	size;
int	scale;
{
	if( filename )
		printf( "%s\t", filename );
	printf( "%ld\n", size/scale );
}
