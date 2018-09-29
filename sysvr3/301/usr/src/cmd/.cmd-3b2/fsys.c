/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:fsys.c	1.8"
/*
 * fsys.c
 *
 * Validate, mount and unmount filesystems. Avoids standard I/O to
 * remain within 3B2 floppy installation space constraints.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/filsys.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	STDERR	2			/* fileno(stderr) */

/*
 * NOTICE: To avoid fire hazard, please remove lint before each load.
 */
#ifdef	lint
#	define	STATIC	static		/* True statics for lint */
#else	/* lint */
#	define	STATIC			/* Global variables go in .bss */
#endif	/* lint */

/*
 * External functions.
 */
void	exit();
long	lseek();
char	*strchr();
char	*strrchr();

/*
 * Internal functions.
 */
void	fatal();
int	getopt();
ushort	octal();
char	*syserr();
void	usage();

/*
 * External variable.
 */
extern int	errno;			/* System error code */
extern char	*sys_errlist[];		/* System error messages */
extern int	sys_nerr;		/* Number of sys_errlist entries */

/*
 * Static variables.
 */
STATIC char	*mountf;		/* Mount filesystem */
STATIC char	*myname;		/* Last qualifier of arg0 */
STATIC ushort	fsmode;			/* New filesystem root permissions */
STATIC char	*fsname;		/* Filesystem name */
STATIC int	optind = 1;		/* Argument index */
STATIC char	*optarg;		/* Option argument */
STATIC int	readonly;		/* Mount read-only */
STATIC int	silent;			/* Shut up about label mismatches */
STATIC int	umountf;		/* Unmount filesystem */
STATIC char	*volume;		/* Volume name */

main(ac, av)
int		ac;
register char	**av;
{
	reg int		fd;
	reg char	*device;
	reg int		c;
	struct filsys	filsys;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	while ((c = getopt(ac, av, "c:f:m:rsuv:")) != -1)
		switch (c) {
		case 'c':
			fsmode = octal(optarg);
			break;
		case 'f':
			fsname = optarg;
			break;
		case 'm':
			mountf = optarg;
			break;
		case 'r':
			++readonly;
			break;
		case 's':
			++silent;
			break;
		case 'u':
			++umountf;
			break;
		case 'v':
			volume = optarg;
			break;
		default:
			usage();
		}
	if (ac - optind != 1)
		usage();
	device = av[optind];
	if (fsname || volume) {
		if ((fd = open(device, O_RDONLY)) < 0
		  || lseek(fd, (long) SUPERBOFF, 0) < 0
		  || read(fd, (char *) &filsys,
		    sizeof(filsys)) != sizeof(filsys))
			fatal(syserr(), device);
		(void) close(fd);
		if ((fsname && strncmp(filsys.s_fname,
		    fsname, sizeof(filsys.s_fname)) != 0)
		  || (volume && strncmp(filsys.s_fpack,
		    volume, sizeof(filsys.s_fpack)) != 0))
			if (silent)
				exit(1);
			else
				fatal("Label mismatch", device);
	}
	if (mountf) {
		/*  When we have more than one fstype for root */
		/*  this will all have to change anyway. */
		if (mount(device, mountf, readonly) < 0)
			fatal(syserr(), device);
		if (fsmode && chmod(mountf, fsmode) < 0)
			fatal(syserr(), mountf);
	}
	if (umountf && umount(device) < 0)
		fatal(syserr(), device);
	exit(0);
	/* NOTREACHED */
}

/*
 * fatal()
 *
 * Print an error message and exit.
 */
STATIC void
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
 * getopt()
 *
 * Parse options. Stolen from libc, with changes
 * to avoid standard I/O.
 */
STATIC int
getopt(ac, av, options)
int		ac;
reg char	**av;
char		*options;
{
	reg int		c;
	reg char	*cp;
	static int	sp = 1;

	if (sp == 1)
		if (optind >= ac
		    || av[optind][0] != '-' || av[optind][1] == '\0')
			return (-1);
		else if (strcmp(av[optind], "--") == 0) {
			optind++;
			return (-1);
		}
	c = av[optind][sp];
	if (c == ':' || (cp = strchr(options, c)) == 0)
		usage();
	if (*++cp == ':') {
		if (av[optind][sp+1] != '\0')
			optarg = &av[optind++][sp+1];
		else if (++optind >= ac)
			usage();
		else
			optarg = av[optind++];
		sp = 1;
	} else {
		if (av[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = 0;
	}
	return (c);
}

/*
 * octal()
 *
 * Convert a string of octal digits to binary.
 */
STATIC ushort
octal(str)
char		*str;
{
	reg char	*idx;
	reg ushort	result = 0;

	for (idx = str; *idx && *idx >= '0' && *idx <= '7'; ++idx)
		result = (result << 3) | (*idx - '0');
	if (*idx)
		fatal("Not an octal number", str);
	return (result);
}

/*
 * syserr()
 *
 * Return a pointer to a system error message.
 */
STATIC char *
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
STATIC void
usage()
{
	static char	before[] = "Usage:\t";
	static char	after[] = " [ -c mode ] [ -f fsname ] [ -m dir ] [ -r ]\n\t[ -s ] [ -u ] [ -v volume ] device\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}
