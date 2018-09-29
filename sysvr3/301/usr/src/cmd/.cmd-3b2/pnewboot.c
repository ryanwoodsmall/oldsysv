/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:pnewboot.c	1.2"
/*
 * Simplified version of newboot. Avoids stdio to help in
 * the first-3B2-restore-floppy space crunch.
 */
/*
 * Write the two boot programs (mboot and lboot) to the disk:
 *
 *	Block 0    ==> mboot program
 *	Block 1    ==> updated VTOC
 *	Block 2... ==> lboot program
 *
 * Usage: pnewboot lboot mboot partition
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <fcntl.h>
#include <a.out.h>

/*
 * Definitions
 */
#define reg 	register	/* Convenience */
#define uint	unsigned int	/* Convenience */
#define STDERR  2		/* Convenience */

/*
 * external routines
 */
extern long lseek();
extern char *strrchr();
extern char *malloc();
extern char *strtok();

/*
 * Internal routines
 */
void 	usage();
int	warn();


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


static	char *myname;

main(argc, argv)
int argc;
char *argv[];
	{
	char	*block0, *block2;
	int	f = -1;
	int	fso = -1;
	long	lboot_size, lboot_origin;
	char	*fsys, *lboot, *mboot;

	register	SCNHDR	*text, *data;
	register FILHDR *header;
	register SCNHDR *sheader;

	if (myname = strrchr(*argv, '/'))
		++myname;
	else	myname = *argv;
	if (argc != 4)
		usage();

	lboot = argv[1];
	mboot = argv[2];
	fsys = argv[3];


	header = (FILHDR *) malloc( FILHSZ );
	sheader = (SCNHDR *) malloc ( SCNHSZ );
	/*
	 * the mboot program is the first block
	 */


	f = open(mboot, O_RDONLY);
	if (f == -1)
		{
		warn(argv[2], "cannot open");
		goto error_exit;
		}

	if ( myread(f, (char *)header, FILHSZ, mboot) ) 
		goto error_exit;

	if ( header->f_magic != FBOMAGIC )
		{
		warn(argv[2], "bad magic");
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
			sheader = (SCNHDR *) malloc( SCNHSZ ) ;
			}
		if ( 0 == strncmp(sheader->s_name,".data",sizeof(sheader->s_name)) )
			{
			data = sheader;
			sheader = (SCNHDR *) malloc( SCNHSZ ) ;
			}
		}

	/*
	 * make sure mboot fits in a single block
	 */
	if ( data->s_paddr + data->s_size - text->s_paddr > BSIZE )
		{
		warn(argv[2], "total size exceeds 1 disk block");
		goto error_exit;
		}



	block0 = malloc ( BSIZE );
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


	free ( text );
	free ( data );
	close( f );

	/*
	 * Now do lboot
	 */

	f = open( lboot, O_RDONLY );
	if (f == -1)
		{
		warn(argv[1], "cannot open");
		goto error_exit;
		}

	if ( myread(f, (char *)header, FILHSZ, lboot) )
		goto error_exit;

	if ( header->f_magic != FBOMAGIC )
		{
		warn(argv[1], "bad magic");
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
			sheader = (SCNHDR *) malloc ( SCNHSZ );
			}
		if ( 0 == strncmp(sheader->s_name,".data",sizeof(sheader->s_name)) )
			{
			data = sheader;
			sheader = (SCNHDR *) malloc ( SCNHSZ );
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

	block2 = malloc ( (unsigned)lboot_size );

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
	 * Open the boot device
	 */
	fso = open(fsys, O_RDWR, 0600 );
	if (fso == -1)
		{
		warn(argv[3], "cannot open");
		goto error_exit;
		}


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
		warn(message, "read error");
		return( 1 );
		}
	else
		if ( rbyte != nbyte )
			{
			warn(message, "truncated read");
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
		warn(message, "write error");
		return( 1 );
		}
	else
		if ( wbyte != nbyte )
			{
			warn(message, "truncated write");
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
		warn(message, "lseek error");
		return( 1 );
		}
	else
		return( 0 );
	}

/*
 * uasage()
 */
static void 
usage()
{
	static char	before[] = "Usage: \t";
	static char	after[] = "  lboot mboot partition\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}

/*
 * warn()
 */
static int
warn(what, why)
reg char	*what;
reg char	*why;
{
	static char	between[] = ": ";
	static char	after[] = "\n";

	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, between, (uint) strlen(between));
	(void) write(STDERR, what, (uint) strlen(what));
	(void) write(STDERR, between, (uint) strlen(between));
	(void) write(STDERR, why, (uint) strlen(why));
	(void) write(STDERR, after, (uint) strlen(after));
}
