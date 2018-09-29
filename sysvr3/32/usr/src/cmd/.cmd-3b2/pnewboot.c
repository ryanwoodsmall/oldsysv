/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:pnewboot.c	1.2.1.2"
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
extern char *malloc();

/*
 * Internal routines
 */
void 	usage();
int	warn();


/*
 * VTOC contains pointers which determine where lboot is to be loaded;
 * it is updated as one entire block (BSIZE long)
 *
 * Any changes here must be coordinated with /usr/src/uts/3b15/boot/mboot/mboot.c
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
	int	fsi = -1;
	int	fso = -1;
	long	lboot_size, lboot_origin;
	char	*fsysin, *fsysout;


	myname = *argv;
	if (argc != 3)
		usage();

	fsysin = argv[1];
	fsysout = argv[2];

	/*
	 * Open the input boot device
	 */
	fsi = open(fsysin, O_RDONLY, 0600 );
	if (fsi == -1)
		{
		warn(fsysin, "cannot open");
		goto error_exit;
		}


	/*
	 * Open the output boot device
	 */
	fso = open(fsysout, O_RDWR, 0600 );
	if (fso == -1)
		{
		warn(fsysout, "cannot open");
		goto error_exit;
		}


	block0 = malloc(BSIZE);

	/*
	 * read the first disk block
	 */
	if ( myread(fsi, block0, BSIZE, fsysin) )
		goto error_exit;

	/*
	 * write the first disk block
	 */
	if ( mywrite(fso, block0, BSIZE, fsysout) )
		goto error_exit;

	/*
	 * read the input VTOC; save the input lboot info
	 */
	if ( myread(fsi, (char *)&block1, BSIZE, fsysin) )
		goto error_exit;

	lboot_origin = block1.vtoc_buffer.v_bootinfo[0];
	lboot_size   = block1.vtoc_buffer.v_bootinfo[1];


	/*
	 * update the output VTOC
	 */
	if ( myread(fso, (char *)&block1, BSIZE, fsysout) )
		goto error_exit;

	block1.vtoc_buffer.v_bootinfo[0] = lboot_origin;
	block1.vtoc_buffer.v_bootinfo[1] = lboot_size;

	if ( mylseek(fso, 0L - BSIZE, 1, fsysout) )
		goto error_exit;

	if ( mywrite(fso, (char *)&block1, BSIZE, fsysout) )
		goto error_exit;

	block2 = malloc(lboot_size);

	/*
	 * read the final disk blocks
	 */
	if ( myread(fsi, block2, (unsigned)lboot_size, fsysin) )
		goto error_exit;

	/*
	 * write the final disk blocks
	 */
	if ( mywrite(fso, block2, (unsigned)lboot_size, fsysout) )
		goto error_exit;

	close(fsi);
	close(fso);

	exit( 0 );

	/*
	 * common error exit
	 */
error_exit:
	if ( fsi >= 0 )
		close( fsi );
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
	static char	after[] = "  from_partition to_partition\n";

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
