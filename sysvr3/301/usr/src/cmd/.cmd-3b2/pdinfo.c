/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:pdinfo.c	1.2"
/*
 * pdinfo.c
 *
 * Print shell assignments for hard disk physical information.
 *
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/vtoc.h>
#include <sys/id.h>
#include <errno.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	DECIMAL	10			/* Numeric base 10 */
#define	STDERR	2			/* Standard error file descriptor */
#define	STDOUT	1			/* Standard output file descriptor */

/*
 * External functions.
 */
void	exit();
char	*strchr();
char	*strrchr();

/*
 * Internal functions.
 */
int	pdinfo();
void	prn();
void	prs();
int	readpd();
void	result();
char	*syserr();
void	usage();
int	warn();

/*
 * External variables.
 */
extern int	errno;			/* System error code */
extern char	*sys_errlist[];		/* Error messages */
extern int	sys_nerr;		/* Number of sys_errlist[] entries */

/*
 * Static variables.
 */
static char	*myname;		/* Last qualifier of arg0 */

main(ac, av)
int		ac;
reg char	**av;
{
	reg ulong	errors = 0;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	if (ac < 2)
		usage();
	while (*++av)
		if (pdinfo(*av) < 0)
			++errors;
	exit(errors ? 1 : 0);
}

/*
 * pdinfo()
 *
 * Print physical information for one device.
 */
static int
pdinfo(name)
char		*name;
{
	reg ulong	cylsize;
	reg ulong	nsector;
	reg int		fd;
	reg int		drive;
	reg int		ok;
	struct stat	sb;
	struct pdsector	pdsector;

	if (stat(name, &sb) < 0)
		return (warn(name, syserr()));
	if ((sb.st_mode & S_IFMT) != S_IFCHR)
		return (warn(name, "Not a raw device"));
	if ((fd = open(name, O_RDONLY)) < 0)
		if (errno == ENXIO)
			return (0);
		else
			return (warn(name, syserr()));
	ok = (readpd(fd, name, &pdsector) == 0);
	(void) close(fd);
	if (!ok)
		return (-1);
	drive = iddn(minor(sb.st_rdev));
	cylsize = pdsector.pdinfo.tracks * pdsector.pdinfo.sectors;
	nsector = pdsector.pdinfo.cyls - 1
	    - (pdsector.pdinfo.logicalst + cylsize - 1) / cylsize;
	result("DRIVEID", drive, pdsector.pdinfo.driveid);
	result("CYLSIZE", drive, cylsize);
	result("NSECTOR", drive, nsector);
	return (0);
}

/*
 * prn()
 *
 * Print a number.
 */
static void
prn(number, base)
reg ulong	number;
reg int		base;
{
	reg char	*idx;
	auto char	buf[64];

	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = "0123456789abcdef"[number % base];
		number /= base;
	} while (number);
	prs(idx);
}

/*
 * prs()
 *
 * Print a string.
 */
static void
prs(str)
reg char	*str;
{
	(void) write(STDOUT, str, (uint) strlen(str));
}

/*
 * readpd()
 *
 * Read physical device information.
 */
static int
readpd(fd, name, pdsector)
int		fd;
reg char	*name;
struct pdsector	*pdsector;
{
	struct io_arg	args;

	args.sectst = 0;
	args.memaddr = (unsigned long) pdsector;
	args.datasz = sizeof(struct pdsector);
	if (ioctl(fd, V_PDREAD, &args) < 0)
		return (warn(name, syserr()));
	if (args.retval == V_BADREAD)
		return (warn(name, "Unable to read device information sector"));
	return (0);
}

/*
 * result()
 *
 * Print a single result.
 */
static void
result(name, drive, value)
char	*name;
int	drive;
ulong	value;
{
	prs(name);
	prs("_");
	prn((ulong) drive, DECIMAL);
	prs("=");
	prn(value, DECIMAL);
	prs("\n");
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
 * usage()
 *
 * Print a helpful message and exit.
 */
static void
usage()
{
	static char	before[] = "Usage:\t";
	static char	after[] = " raw_disk_device ...\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}

/*
 * warn()
 *
 * Print an error message. Always returns -1.
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
	return (-1);
}
