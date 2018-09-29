/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vmkfs:vmkfs.c	1.2"
/*
 * vmkfs.c
 *
 * Make a filesystem within a hard disk partition. Avoids the
 * standard I/O library to remain within first-floppy space
 * constraints.
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vtoc.h>
#include <sys/sysmacros.h>
#include <sys/id.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	STDERR	2			/* Standard error file descriptor */
#define	STDOUT	1			/* Standard output file descriptor */

/*
 * External functions.
 */
void	exit();
char	*strchr();
char	*strcpy();
char	*strrchr();

/*
 * Internal functions.
 */
void	child();
void	fatal();
char	*itoa();
int	readpd();
int	readvtoc();
char	*syserr();
void	usage();
int	vmkfs();
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
static short	nflag;			/* Don't execute mkfs(1M) */
static short	xflag;			/* Trace generated mkfs(1M) commands */
static char	*myname;		/* Last qualifier of arg0 */

main(ac, av)
int		ac;
reg char	**av;
{
	reg int		ret;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	while (*++av && **av == '-')
		if (strcmp(*av, "-n") == 0)
			++nflag;
		else if (strcmp(*av, "-x") == 0)
			++xflag;
		else
			fatal(*av, "Unknown option");
	if (*av == 0)
		usage();
	for (ret = 0; *av; ++av)
		ret |= vmkfs(*av);
	exit(ret);
}

/*
 * child()
 *
 * This is the forked child. Execute mkfs(1M) with
 * appropriate parameters. NEVER returns.
 */
static void
child(name, pdinfo, part)
reg char	*name;
reg struct pdinfo	*pdinfo;
reg struct partition	*part;
{
	auto char	blocks[64];
	auto char	cylsize[64];
	auto char	*args[6];

	args[0] = "/etc/mkfs";
	args[1] = name;
	args[2] = strcpy(blocks, itoa((ulong) part->p_size));
	args[3] = "10";
	args[4] = strcpy(cylsize, itoa(pdinfo->tracks * pdinfo->sectors));
	args[5] = 0;
	if (xflag) {
		reg char	**av;

		(void) write(STDOUT, "++", 2);
		for (av = args; *av; ++av) {
			(void) write(STDOUT, " ", 1);
			(void) write(STDOUT, *av, (uint) strlen(*av));
		}
		(void) write(STDOUT, "\n", 1);
	}
	if (nflag)
		exit(0);
	(void) execv(args[0], args);
	fatal(name, syserr());
}

/*
 * fatal()
 *
 * Print an error message and exit.
 */
static void
fatal(what, why)
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

/*
 * itoa()
 *
 * Convert an integer to a null-terminated ASCII
 * string. Returns a pointer to the static result.
 */
static char *
itoa(number)
reg ulong	number;
{
	reg char	*idx;
	static char	buf[64];

	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = '0' + number % 10;
		number /= 10;
	} while (number);
	return (idx);
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
 * readvtoc()
 *
 * Read a partition map.
 */
static int
readvtoc(fd, name, pdinfo, vtoc)
int		fd;
char		*name;
struct pdinfo	*pdinfo;
struct vtoc	*vtoc;
{
	struct io_arg	args;

	args.sectst = pdinfo->logicalst + 1;
	args.memaddr = (unsigned long) vtoc;
	args.datasz = sizeof(struct vtoc);
	if (ioctl(fd, V_PREAD, &args) < 0)
		return (warn(name, syserr()));
	if (args.retval == V_BADREAD)
		return (warn(name, "Unable to read VTOC"));
	if (vtoc->v_sanity != VTOC_SANE)
		return (warn(name, "No valid VTOC on device"));
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
 * usage()
 *
 * Print a helpful message and exit.
 */
static void
usage()
{
	static char	before[] = "Usage:\t";
	static char	after[] = " [ -v ] raw_disk_device ...\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}

/*
 * vmkfs()
 *
 * Get device information and make a filesystem. Quietly skips
 * unmountable and zero-length partitions.
 */
static int
vmkfs(name)
char		*name;
{
	reg struct partition	*part;
	reg int		fd;
	reg int		pid;
	reg int		ok;
	auto int	status;
	struct stat	sb;
	struct vtoc	vtoc;
	struct pdsector	pdsector;

	if (stat(name, &sb) < 0)
		return (warn(name, syserr()));
	if ((sb.st_mode & S_IFMT) != S_IFCHR)
		return (warn(name, "Not a raw device"));
	if ((fd = open(name, O_RDONLY)) < 0)
		return (warn(name, syserr()));
	ok = (readpd(fd, name, &pdsector) == 0
	    && readvtoc(fd, name, &pdsector.pdinfo, &vtoc) == 0);
	(void) close(fd);
	if (!ok)
		return (-1);
	part = &vtoc.v_part[idslice(minor(sb.st_rdev))];
	if (part->p_size == 0 || part->p_flag & V_UNMNT)
		return (0);
	if ((pid = fork()) < 0)
		return (warn(name, syserr()));
	if (pid == 0)
		child(name, &pdsector.pdinfo, part);
	while (wait(&status) != pid)
		;
	return (status ? warn(name, "mkfs failed") : 0);
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
