/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)newboot:newboot.c	1.2"
/*
 * Write the two boot programs (mboot and lboot) to the disk:
 *
 *	Block 0    ==> mboot program
 *	Block 1    ==> updated VTOC
 *	Block 2... ==> lboot program
 *
 * Usage: newboot lboot mboot filsys
 *
 * Before writing the filesys, a confirmation message is issued if the -y
 * flag was not present.  The response must be "y" or "Y", or the program
 * aborts.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <fcntl.h>
#include <a.out.h>

/*
 * external routines
 */
extern char *fgets();
extern long lseek();
extern char *malloc();
extern char *strtok();

/*
 * VTOC contains pointers which determine where lboot is to be loaded;
 * it is updated as one entire block (BSIZE long)
 *
 * Any changes here must be coordinated with /usr/src/uts/3b5/boot/mboot/mboot.c
 */
union	{
	struct vtoc	vtoc_buffer;
	char		buffer[BSIZE];
	}
	block1;

extern int optind;
extern char *optarg;

main(argc, argv)
int argc;
char *argv[];
	{
	char	*block0, *block2;
	char	*fsys, *lboot, *mboot;
	char	*error_message = NULL;
	char	line[81], *p;
	int	c;
	int	f = -1;
	int	fso = -1;
	int	yes = 0;			/* -y option */
	int	error = 0;			/* argument error */
	long	lboot_size, lboot_origin;

	register FILHDR *header;
	register SCNHDR *text, *data, *sheader;

	while( (c=getopt(argc,argv,"y")) != EOF )
		switch( c )
			{
		case 'y':
			yes = 1;
			break;
		case '?':
			error = 1;
			break;
			}

	if ( error || argc-optind < 3 )
		{
		fprintf( stderr, "Usage: %s [-y] lboot mboot partition\n", argv[0] );
		exit(1);
		}

	lboot = argv[optind++];
	mboot = argv[optind++];
	fsys = argv[optind];

	header = (FILHDR *) malloc( FILHSZ );
	sheader = (SCNHDR *) malloc( SCNHSZ );

	/*
	 * the mboot program is the first block
	 */

	error_message = "boot not written";

	f = open(mboot, O_RDONLY);
	if (f == -1)
		{
		perror( mboot );
		goto error_exit;
		}

	if ( myread(f, (char *)header, FILHSZ, mboot) ) 
		goto error_exit;

	if ( header->f_magic != FBOMAGIC )
		{
		fprintf( stderr, "%s: bad magic\n", mboot );
		goto error_exit;
		}

	if ( mylseek(f, (long)(header->f_opthdr), 1, mboot) )
		goto error_exit;

	while( header->f_nscns-- > 0 )
		{
		if ( myread(f, (char *)sheader, SCNHSZ, mboot) )
			goto error_exit;
		if ( 0 == strncmp(sheader->s_name,".text",sizeof(sheader->s_name)) )
			{
			text = sheader;
			sheader = (SCNHDR *) malloc( SCNHSZ );
			}
		if ( 0 == strncmp(sheader->s_name,".data",sizeof(sheader->s_name)) )
			{
			data = sheader;
			sheader = (SCNHDR *) malloc( SCNHSZ );
			}
		}

	/*
	 * make sure mboot fits in a single block
	 */
	if ( data->s_paddr + data->s_size - text->s_paddr > BSIZE )
		{
		fprintf( stderr, "%s: total size (0x%x) exceeds 1 disk block\n",
				 mboot,
				 data->s_paddr + data->s_size - text->s_paddr );
		goto error_exit;
		}

	block0 = malloc ( BSIZE );

	if ( block0 == NULL )
		{
		fprintf( stderr, "%s: out of memory\n", mboot );
		goto error_exit;
		}

	/*
	 * build the first disk block
	 */
	if ( mylseek(f, text->s_scnptr, 0, mboot) )
		goto error_exit;
	if ( myread(f, block0, (unsigned)text->s_size, mboot) )
		goto error_exit;
	if ( mylseek(f, data->s_scnptr, 0, mboot) )
		goto error_exit;
	if ( myread(f, block0 + (data->s_paddr - text->s_paddr), (unsigned)data->s_size, mboot) )
		goto error_exit;

	free( text );
	free( data );

	close( f );

	/*
	 * Now do lboot
	 */

	f = open( lboot, O_RDONLY );
	if (f == -1)
		{
		perror( lboot );
		goto error_exit;
		}

	if ( myread(f, (char *)header, FILHSZ, lboot) )
		goto error_exit;

	if ( header->f_magic != FBOMAGIC )
		{
		fprintf( stderr, "%s: bad magic\n", lboot );
		goto error_exit;
		}

	if ( mylseek(f, (long)(header->f_opthdr), 1, lboot) )
		goto error_exit;

	while( header->f_nscns-- > 0 )
		{
		if ( myread(f, (char *)sheader, SCNHSZ, lboot) )
			goto error_exit;
		if ( 0 == strncmp(sheader->s_name,".text",sizeof(sheader->s_name)) )
			{
			text = sheader;
			sheader = (SCNHDR *) malloc( SCNHSZ );
			}
		if ( 0 == strncmp(sheader->s_name,".data",sizeof(sheader->s_name)) )
			{
			data = sheader;
			sheader = (SCNHDR *) malloc( SCNHSZ );
			}
		}

	/*
	 * compute lboot size and load origin
	 */
	lboot_origin = text->s_paddr;

	/* size of lboot in bytes (rounded up to nearest BSIZE) */
	lboot_size = data->s_paddr + data->s_size - text->s_paddr;
	lboot_size = ( (lboot_size + BSIZE - 1) / BSIZE) * BSIZE;

	/*
	 * get a buffer large enough to build the lboot program
	 */
	block2 = malloc( (unsigned)lboot_size );

	if ( block2 == NULL )
		{
		fprintf( stderr, "%s: out of memory\n", lboot );
		goto error_exit;
		}

	/*
	 * build the lboot program buffer
	 */
	if ( mylseek(f, text->s_scnptr, 0, lboot) )
		goto error_exit;
	if ( myread(f, block2, (unsigned)text->s_size, lboot) )
		goto error_exit;
	if ( mylseek(f, data->s_scnptr, 0, lboot) )
		goto error_exit;
	if ( myread(f, block2 + (data->s_paddr - text->s_paddr), (unsigned)data->s_size, lboot) )
		goto error_exit;

	close(f);
	f = -1;

	/*
	 * Write the boot partition after receiving confirmation
	 */

	if ( ! yes )
		{
		do	{
			printf( "%s: confirm request to write boot programs to %s: ", argv[0], fsys );
			}
			while( fgets(line,sizeof(line),stdin) == NULL || (p=strtok(line," \t\n\r")) == NULL );

		if ( *p != 'y' && *p != 'Y' )
			goto error_exit;
		}

	/*
	 * Open the boot device
	 */
	fso = open(fsys, O_RDWR, 0600 );
	if (fso == -1)
		{
		perror( fsys );
		goto error_exit;
		}

	error_message = "boot partition destroyed!";

	/*
	 * write the first disk block
	 */
	if ( mywrite(fso, block0, BSIZE, fsys) )
		goto error_exit;

	/*
	 * update the VTOC
	 */
	if ( myread(fso, (char *)&block1, BSIZE, fsys) )
		goto error_exit;

	block1.vtoc_buffer.v_bootinfo[0] = lboot_origin;
	block1.vtoc_buffer.v_bootinfo[1] = lboot_size;

	if ( mylseek(fso, 0L - BSIZE, 1, fsys) )
		goto error_exit;

	if ( mywrite(fso, (char *)&block1, BSIZE, fsys) )
		goto error_exit;

	/*
	 * write the final disk blocks
	 */
	if ( mywrite(fso, block2, (unsigned)lboot_size, fsys) )
		goto error_exit;

	close(fso);

	exit( 0 );

	/*
	 * common error exit
	 */
error_exit:
	if ( f >= 0 )
		close( f );
	if ( fso >= 0 )
		close( fso );

	fprintf( stderr, "%s: %s\n", argv[0], error_message );

	exit( 1 );
	}

/*
 * Read with error checking
 */
 int
myread( fildes, buf, nbyte, message )
	int fildes;
	char *buf;
	register unsigned nbyte;
	register char *message;
	{
	register int rbyte;

	if ( (rbyte = read(fildes,buf,nbyte)) == -1 )
		{
		perror( message );
		return( 1 );
		}
	else
		if ( rbyte != nbyte )
			{
			fprintf( stderr, "%s: truncated read\n", message );
			return( 1 );
			}
		else
			return( 0 );
	}

/*
 * Write with error checking
 */
 int
mywrite( fildes, buf, nbyte, message )
	int fildes;
	char *buf;
	register unsigned nbyte;
	register char *message;
	{
	register int wbyte;

	if ( (wbyte = write(fildes,buf,nbyte)) == -1 )
		{
		perror( message );
		return( 1 );
		}
	else
		if ( wbyte != nbyte )
			{
			fprintf( stderr, "%s: truncated write\n", message );
			return( 1 );
			}
		else
			return( 0 );
	}

/*
 * Lseek with error checking
 */
 int
mylseek( fildes, offset, whence, message )
	int fildes;
	long offset;
	int whence;
	char *message;
	{

	if ( lseek(fildes,offset,whence) == -1L )
		{
		perror( message );
		return( 1 );
		}
	else
		return( 0 );
	}
