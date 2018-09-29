/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:dswap.c	1.2"
/*
 * swap.c
 *
 * Connect a hard disk partition to the running kernel for swapping.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/sys3b.h>
#include <sys/vtoc.h>
#include <errno.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	STDERR	2			/* fileno(stderr) */

/*
 * External functions.
 */
void	exit();
char	*strcat();
char	*strrchr();
char	*strcpy();

/*
 * Internal functions.
 */
void	fatal();
int	findswap();
int	pdread();
int	pread();
char	*syserr();
int	toblock();
void	usage();

/*
 * External variables.
 */
extern int	errno;			/* System error code */
extern char	*sys_errlist[];		/* System error messages */
extern int	sys_nerr;		/* Number of sys_errlist entries */

/*
 * Static variables.
 */
static char	*myname;		/* Last qualifier of arg0 */

main(ac, av)
int	ac;
char	**av;
{
	reg int		fd;
	reg int		partno;
	struct pdsector	pdsector;
	struct vtoc	vtoc;
	char		bdev[80];

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	if (ac != 2)
		usage();
	if (toblock(av[1], bdev) < 0)
		fatal("Must follow device naming conventions!", av[1]);
	if ((fd = open(av[1], O_RDONLY)) < 0
	  || pdread(fd, &pdsector) < 0
	  || pread(fd, (daddr_t) pdsector.pdinfo.logicalst + 1,
	      (caddr_t) &vtoc, (daddr_t) sizeof(struct vtoc)) < 0)
		fatal(syserr(), av[1]);
	(void) close(fd);
	if (vtoc.v_sanity != VTOC_SANE)
		fatal("Invalid VTOC", av[1]);
	if ((partno = findswap(vtoc.v_part, vtoc.v_nparts)) < 0)
		fatal("No swap partition on device", av[1]);
	bdev[strlen(bdev) - 1] = "0123456789abcdef"[partno];
	if (sys3b(S3BSWAP, bdev, 0, vtoc.v_part[partno].p_size) < 0)
		fatal(syserr(), bdev);
	exit(0);
}

/*
 * fatal()
 *
 * Print an error message and exit.
 */
static void
fatal(why, what)
reg char	*why;
reg char	*what;
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

/*
 * findswap()
 *
 * Look for a valid swap partition.
 */
static int
findswap(ptab, nparts)
struct partition	*ptab;
ushort			nparts;
{
	reg struct partition	*pidx;
	reg struct partition	*pend = ptab + nparts;

	for (pidx = ptab; pidx < pend; ++pidx)
		if (pidx->p_size
		    && pidx->p_tag == V_SWAP
		    && (pidx->p_flag & (V_UNMNT | V_RONLY)) == V_UNMNT)
			return (pidx - ptab);
	return (-1);
}

/*
 * pdread()
 *
 * Read the disk's physical description.
 */
static int
pdread(fd, pdsector)
int		fd;
struct pdsector	*pdsector;
{
	struct io_arg	args;

	args.sectst = 0;
	args.memaddr = (ulong) pdsector;
	args.datasz = 0;
	if (ioctl(fd, V_PDREAD, &args) < 0)
		return (-1);
	if (args.retval == V_BADREAD) {
		errno = EIO;
		return (-1);
	}
	return (0);
}

/*
 * pread()
 *
 * Physical read.
 */
static int
pread(fd, sectst, memaddr, datasz)
int	fd;
daddr_t	sectst;
caddr_t	memaddr;
daddr_t	datasz;
{
	struct io_arg	args;

	args.sectst = (ulong) sectst;
	args.memaddr = (ulong) memaddr;
	args.datasz = (ulong) datasz;
	if (ioctl(fd, V_PREAD, &args) < 0)
		return (-1);
	if (args.retval == V_BADREAD) {
		errno = EIO;
		return (-1);
	}
	return (0);
}

/*
 * syserr()
 *
 * Return a pointer to a system error message.
 */
static char *
syserr()
{
	return (errno <= 0 ? "No error (?)"
	    : errno < sys_nerr ? sys_errlist[errno]
	    : "Unknown error (!)");
}

/*
 * toblock()
 *
 * Convert a raw disk device name into the corresponding block device
 * name. Handles both Sys 5 and Sys 5 Rel 2 naming conventions.
 */
static int
toblock(raw, block)
char	*raw;
char	*block;
{
	static char	prefix[] = "/dev/r";

	if (strncmp(raw, prefix, sizeof(prefix) - 1) != 0)
		return (-1);
	(void) strcat(strcpy(block, "/dev/"), raw + sizeof(prefix) - 1);
	return (0);
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
	static char	after[] = " raw_device\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}
