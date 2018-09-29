/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)disks:disks.c	1.4"
/*
 * disks.c
 *
 * Set up "/dev" nodes for equipped hard disks. Avoids standard
 * I/O to aid in the never-ending battle for space on the first
 * 3B2 restore floppy.
 *
 * The system board ("SBD") holds the integral floppy and hard
 * disk controllers. Their drivers share a major device number.
 * Unfortunately, for compatibility with released software, the
 * floppy and hard disks have different controller numbers (i.e.,
 * "/dev/[r]dsk/c<controller>d?s?"). This gap in numbers is sized
 * by SBDXDEV.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/sysmacros.h>
#include <sys/edt.h>
#include <sys/sys3b.h>

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	ACCESS	0600			/* Device permissions */
#define	DECIMAL	10			/* Numeric base 10 */
#define	HEX	16			/* Numeric base 16 */
#define	NAMELEN	40			/* Maximum device name length */
#define	PARTALL	6			/* "Partition" mapping to entire disk */
#define	PARTCNT	16			/* Number of disk partitions */
#define	SBDMAJOR 17			/* Offset of SBD major from slot */
#define	SBDSKIP	1			/* SBD device(s) to skip (such as IF) */
#define	SBDXDEV	1			/* Extra major numbers used by SBD */
#define	STDERR	2			/* Standard error file descriptor */
#define	STDOUT	1			/* Standard output file descriptor */

/*
 * Simple types.
 */
typedef struct edt	Edt;		/* Equipped-device table (EDT) entry */
typedef struct stat	Stat;		/* stat(2) structure */
typedef struct subdevice Sub;		/* EDT subdevice entry */

/*
 * Structure returned by sys3b(S3BEDT) at the beginning
 * of the data.
 */
typedef struct {
	short		size_edt;	/* Number of EDT entries */
	short		size_sub;	/* Number of subdevice entries */
} Edtsize;

/*
 * External functions.
 */
void	exit();
char	*malloc();
int	(*signal())();
char	*strcat();
char	*strcpy();
char	*strrchr();

/*
 * Internal functions.
 */
void	devneed();
char	*devsablk();
char	*devsanxt();
char	*devsaraw();
char	*devxblk();
char	*devxraw();
void	edtget();
void	fatal();
int	itoa();
void	prn();
void	prs();
char	*syserr();
void	warn();

/*
 * External variables.
 */
extern int	errno;			/* System error code */
extern char	*sys_errlist[];		/* Error messages */
extern int	sys_nerr;		/* Number of sys_errlist[] entries */

/*
 * Static variables.
 */
static Edt	*edtbgn;		/* Start of equipped-device table */
static Edt	*edtend;		/* End of equipped-device table */
static char	*myname;		/* Last qualifier of arg0 */

main(ac, av)
int	ac;
char	**av;
{
	reg Edt		*edt;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	/* (void) signal(SIGQUIT, SIG_IGN); */
	(void) signal(SIGTERM, SIG_IGN);
	edtget();
	for (edt = edtbgn; edt < edtend; ++edt)
		if (strcmp(edt->dev_name, "SBD") == 0)
			devneed(edt->opt_slot + SBDMAJOR,
			    edt->opt_slot + SBDXDEV,
			    edt->subdev + SBDSKIP,
			    edt->n_subdev - SBDSKIP);
		else if (strcmp(edt->dev_name, "XDC") == 0)
			devneed(edt->opt_slot,
			    edt->opt_slot + SBDXDEV,
			    edt->subdev,
			    edt->n_subdev);
	exit(0);
	/* NOTREACHED */
}

/*
 * devneed()
 *
 * Check for (and possibly create) a set of "/dev" entries. Assumes
 * that a nonzero subdevice "opt_code" (drive ID) indicates a valid
 * disk drive.
 */
static void
devneed(maj, ctl, sub, nsub)
uint		maj;
uint		ctl;
reg Sub		*sub;
uint		nsub;
{
	reg uint	drive;
	reg uint	slice;
	reg char	*name;
	reg int		haveblk;
	reg int		haveraw;
	reg int		count;
	reg char	*sa;
	auto Stat	sb;

	count = 0;
	for (drive = 0; drive < nsub; ++drive) {
		if (sub[drive].opt_code == 0)
			continue;
		haveblk = (stat(devxblk(ctl, drive, PARTALL), &sb) == 0);
		haveraw = (stat(devxraw(ctl, drive, PARTALL), &sb) == 0);
		if (haveblk && haveraw)
			continue;
		sa = devsanxt();
		if (++count == 1)
			prs("\n");
		prs("The new disk in hardware slot ");
		prn(ctl - 1, DECIMAL, 0);
		prs(" drive ");
		prn(drive, DECIMAL, 0);
		prs(" has been labeled \"");
		prs(sa);
		prs("\".\n");
		for (slice = 0; slice < PARTCNT; ++slice) {
			if (!haveblk && mknod(name = devxblk(ctl, drive, slice),
			    S_IFBLK | ACCESS,
			    makedev(maj, drive * PARTCNT + slice)) < 0)
				warn(name, syserr());
			if (!haveraw && mknod(name = devxraw(ctl, drive, slice),
			    S_IFCHR | ACCESS,
			    makedev(maj, drive * PARTCNT + slice)) < 0)
				warn(name, syserr());
		}
		if (link(devxblk(ctl, drive, PARTALL), name = devsablk(sa)) < 0)
			warn(name, syserr());
		if (link(devxraw(ctl, drive, PARTALL), name = devsaraw(sa)) < 0)
			warn(name, syserr());
	}
	if (count) {
		prs("Use \"sysadm partitioning\" to configure ");
		prs(count == 1 ? "it.\n" : "them.\n");
	}
}

/*
 * devsablk()
 *
 * Name a block SA device.
 */
static char *
devsablk(suffix)
char		*suffix;
{
	static char	name[NAMELEN];

	return (strcat(strcpy(name, "/dev/SA/"), suffix));
}

/*
 * devsanxt()
 *
 * Find the first available SA device name.
 */
static char *
devsanxt()
{
	reg uint	seq;
	auto Stat	sb;
	static char	name[NAMELEN];
	static char	prefix[] = "disk";

	seq = 0;
	(void) strcpy(name, prefix);
	do {
		++seq;
		(void) itoa(name + sizeof(prefix) - 1, seq, DECIMAL, 0);
	} while (stat(devsablk(name), &sb) == 0
	    || stat(devsaraw(name), &sb) == 0);
	return (name);
}

/*
 * devsaraw()
 *
 * Name a raw SA device.
 */
static char *
devsaraw(suffix)
char		*suffix;
{
	static char	name[NAMELEN];

	return (strcat(strcpy(name, "/dev/rSA/"), suffix));
}

/*
 * devxblk()
 *
 * Name a block device.
 */
static char *
devxblk(ctl, drive, slice)
uint		ctl;
uint		drive;
uint		slice;
{
	reg char	*idx;
	static char	name[NAMELEN];
	static char	prefix[] = "/dev/dsk/c";

	idx = strcpy(name, prefix) + sizeof(prefix) - 1;
	idx += itoa(idx, ctl, DECIMAL, 0);
	*idx++ = 'd';
	idx += itoa(idx, drive, DECIMAL, 0);
	*idx++ = 's';
	idx += itoa(idx, slice, HEX, 0);
	*idx = '\0';
	return (name);
}

/*
 * devxraw()
 *
 * Name a raw device.
 */
static char *
devxraw(ctl, drive, slice)
uint		ctl;
uint		drive;
uint		slice;
{
	reg char	*idx;
	static char	name[NAMELEN];
	static char	prefix[] = "/dev/rdsk/c";

	idx = strcpy(name, prefix) + sizeof(prefix) - 1;
	idx += itoa(idx, ctl, DECIMAL, 0);
	*idx++ = 'd';
	idx += itoa(idx, drive, DECIMAL, 0);
	*idx++ = 's';
	idx += itoa(idx, slice, HEX, 0);
	*idx = '\0';
	return (name);
}

/*
 * edtget()
 *
 * Read the equipped-device table.
 */
static void
edtget()
{
     	reg Edt		*edt;
	reg Sub		*sub;
	reg char	*mem;
	reg uint	memsize;
	auto Edtsize	edtsize;

	if (sys3b(S3BEDT, &edtsize, sizeof(int)) < 0)
		fatal("sys3b(S3BEDT, <size>)", syserr());
	memsize = sizeof(Edtsize)
	    + edtsize.size_edt * sizeof(Edt)
	    + edtsize.size_sub * sizeof(Sub);
	if ((mem = malloc(memsize)) == 0)
		fatal("malloc()", "Insufficient memory");
     	if (sys3b(S3BEDT, mem, memsize) < 0)
		fatal("sys3b(S3BEDT, <data>)", syserr());
	edtbgn = (Edt *) (mem + sizeof(Edtsize));
	edtend = edtbgn + edtsize.size_edt;
	sub = (Sub *) edtend;
	for (edt = edtbgn; edt < edtend; ++edt)
		if (edt->n_subdev) {
			edt->subdev = sub;
			sub += edt->n_subdev;
		} else
			edt->subdev = 0;
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
 * Convert an integer to an ASCII string of any base from 2 (binary)
 * to 16 (hexadecimal). Places the result in a specified buffer and
 * returns a pointer to the terminating null character.
 */
static int
itoa(str, number, base, len)
reg char	*str;
reg uint	number;
reg int		base;
ushort		len;
{
	reg char	*idx;
	auto char	buf[NAMELEN];

	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = "0123456789abcdef"[number % base];
		number /= base;
	} while (number);
	number = buf + sizeof(buf) - 1 - idx;
	for ( ; number < len; ++number)
		*--idx = ' ';
	while (*idx)
		*str++ = *idx++;
	*str = '\0';
	return (number);
}

/*
 * prn()
 *
 * Print a number.
 */
static void
prn(number, base, len)
uint		number;
int		base;
ushort		len;
{
	auto char	buf[NAMELEN];

	(void) itoa(buf, number, base, len);
	prs(buf);
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
 * warn()
 *
 * Print a warning message.
 */
static void
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
