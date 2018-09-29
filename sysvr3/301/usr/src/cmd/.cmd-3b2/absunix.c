/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:absunix.c	1.2"
/*
 * absunix.c
 *
 * Zap the <EDT> section of a mkunix'ed kernel. This allows
 * lboot to load it regardless of hardware configuration.
 */

#include <fcntl.h>
#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>

/*
 * Definitions.
 */
#define reg	register		/* Convenience */
#define	uint	unsigned int		/* Convenience */
#define	ulong	unsigned long		/* Convenience */
#define	ushort	unsigned short		/* Convenience */
#define	EDTNAME	"<EDT>"			/* Section to be zapped */
#define	EDTZAP	"<...>"			/* New name to hide "zapped" section */
#define	STDERR	2			/* fileno(stderr) */

/*
 * External functions.
 */
void	exit();
long	lseek();
char	*strrchr();
char	*strncpy();

/*
 * Internal functions.
 */
void	fatal();
char	*syserr();
void	usage();

/*
 * External variables.
 */
extern int	errno;			/* System error number */
extern char	*sys_errlist[];		/* System error messages */
extern int	sys_nerr;		/* Number of sys_errlist entries */

/*
 * Static variables.
 */
static char	*myname;		/* Last qualifier of arg0 */

main(ac, av)
char	**av;
int	ac;
{
	reg int		idx;
	reg int		fd;
	struct filehdr	file;
	struct scnhdr	sec;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	if (ac != 2)
		usage();
	if ((fd = open(av[1], O_RDWR)) < 0)
		fatal(syserr(), av[1]);
	if (read(fd, (char *) &file, sizeof(file)) != sizeof(file)
	    || lseek(fd, (long) file.f_opthdr, 1) < 0)
		fatal(syserr(), av[1]);
	for (idx = 0; idx < file.f_nscns; ++idx) {
		if (read(fd, (char *) &sec, sizeof(sec)) != sizeof(sec))
			fatal(syserr(), av[1]);
		if (strncmp(sec.s_name, EDTNAME, sizeof(sec.s_name)) != 0)
			continue;
		(void) strncpy(sec.s_name, EDTZAP, sizeof(sec.s_name));
		if (lseek(fd, (long) -sizeof(sec), 1) < 0
		    || write(fd, (char *) &sec, sizeof(sec)) != sizeof(sec))
			fatal(syserr(), av[1]);
	}
	(void) close(fd);
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
 * syserr()
 *
 * Return pointer to system error message.
 */
static char *
syserr()
{
	return (errno < 0 ? "Unknown error (?)"
	    : errno == 0 ? "EOF"
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
	static char	before[] = "Usage: ";
	static char	after[] = " mkunix'ed_kernel\n";

	(void) write(STDERR, before, (uint) strlen(before));
	(void) write(STDERR, myname, (uint) strlen(myname));
	(void) write(STDERR, after, (uint) strlen(after));
	exit(1);
}
