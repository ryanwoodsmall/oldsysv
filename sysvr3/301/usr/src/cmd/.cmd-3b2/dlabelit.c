/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:dlabelit.c	1.2"
/*
 * Reads and writes disk file system labels. Avoids stdio to help in
 * the first-3B2-restore-floppy space crunch.
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/filsys.h>
#define DEV 1
#define FSNAME 2
#define VOLUME 3
#define ALL 0
#define SIX 6


/*
 * Definitions.
 */
#define	reg	register	/* Convenience */
#define uint	unsigned int	/* Convenience */
#define ulong	unsigned long	/* Convenience */
#define STDERR	2		/* Convenience */
#define STDOUT  1		/* Convenience */





/*
 * External functions.
 */
void	exit();
long	lseek();
char	*strrchr();

/*
 * Internal functions.
 */
void	prn();
void	prs();
void	usage();
int	warn();
void	cpystr();

/*
 * Static variables.
 */
static char	*myname;		/* Last qualifier of arg0 */

/*
 * Read and Write on disk superblock.
 */
struct {
	char fill1[SUPERBOFF];
	struct filsys fs;
} super;


main(argc, argv) char **argv; {
	reg int		fsi, fso;
	reg int		fs_len ;
	reg int		vol_len ;
	reg int		bsize;
	reg int		i;
	if (myname = strrchr(*argv, '/'))
		++myname;
	else
		myname = *argv;
	if (argc!=4 && argc!=2)
		usage();



	if ((fsi = open(argv[DEV], 0)) < 0)
		warn (argv[DEV], " cannot open device");
	if ((fsi = read(fsi, &super, sizeof(super))) != sizeof(super)) 
		warn (argv[DEV], "cannot read superblock");
	if (super.fs.s_magic != FsMAGIC)
		warn (argv[DEV], "Not a filesystem");
	switch (super.fs.s_type) {
	case  Fs1b:
		bsize = 512 ;
		break;
	case  Fs2b:
		bsize = 1024 ;
		break;
	default:
		warn(argv[DEV], "Unknown filesystem type");
	}
	prs("Current fsname: ",ALL);
	prs(super.fs.s_fname,SIX);
	prs(", Current volname: ",ALL);
	prs(super.fs.s_fpack,SIX);
	prs(", Blocks: ",ALL);
	prn(bsize * super.fs.s_fsize / 512);
	prs(", Inodes: ",ALL);
	prn(bsize * ((super.fs.s_isize - 2) * 8) / 512);
	if (super.fs.s_type == Fs1b)
		prs("\nFS Units: 512b",ALL);
        else	
		prs("\nFS Units: 1kb",ALL);
	prs(", Date last mounted: ",ALL);
	prs(ctime(&super.fs.s_time), ALL);
	if (argc == 2)
		exit(0);
	prs("NEW fsname = ",ALL);
	if ((strlen(argv[FSNAME])) > SIX)
		fs_len = SIX;
	else	fs_len = strlen(argv[FSNAME]);
	prs(argv[FSNAME], fs_len);
	prs(", NEW volname = ",ALL);
	if ((strlen(argv[VOLUME])) > SIX)
		vol_len = SIX;
	else	vol_len = strlen(argv[VOLUME]);
	prs(argv[VOLUME], vol_len);
	prs(" -- DEL if wrong!!\n",ALL);
	sleep(10);
	cpystr(super.fs.s_fname, argv[FSNAME], fs_len);
	cpystr(super.fs.s_fpack, argv[VOLUME], vol_len);

	close(fsi);
	fso = open(argv[DEV], 1);
	if (write (fso,  &super, sizeof(super)) < 0) 
		warn(argv[DEV]," cannot write label");
	exit(0);
}


/*
 * prn()
 *
 * Print a decimal number with a trailing newline.
 */
static void
prn(number)
reg ulong	number;
{
	reg char	*idx;
	auto char	buf[64];

	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = "0123456789abcdef"[number % 10];
		number /= 10;
	} while (number);
	prs(idx,ALL);
}

/*
 * prs()
 *
 * Print a string.
 */
static void
prs(str,len)
reg char	*str;
reg int		len;
{
	register char *os1 = str;
	register uint n = 0;
	if (len == ALL)
		(void) write(STDOUT, str, (uint) strlen(str));
	else
		{
		while ( (*os1++ != '\0') && (++n < SIX) )
			;
		(void) write(STDOUT, str, n);
		}
}


/*
 * cpystr()
 *
 * Copy a string.
 */
static void
cpystr(s1, s2, n)
register char *s1, *s2;
register int n;
{
	register char *os1 = s1;
	register int i;
	
	for (i=1; i<=SIX; i++)
		*os1++ = '\0';
	while (--n >= 0)
		*s1++ = *s2++;
}


/*
 * usage()
 *
 * Print a helpful message and exit.
 */
static void
usage()
{
	static char	before[] = "Usage:\t";
	static char	after[] = " /dev/{r}??? [fsname volume]\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}

/*
 * warn()
 *
 * Print an error message.
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
	exit(1);
}
