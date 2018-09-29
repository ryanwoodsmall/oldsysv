/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:ttyset.c	1.2"
/*
 * ttyset.c
 *
 * Set reasonable tty modes.
 */

#include <termio.h>

/*
 * Macros.
 */
#define	ctl(c)	((c) & ~0140)		/* ASCII "control" character */

/*
 * Definitions.
 */
#define	reg	register		/* Convenience */
#define	STDIN	0			/* fileno(stdin) */
#define	STDERR	2			/* fileno(stderr) */

/*
 * External functions.
 */
void	exit();
char	*strrchr();

/*
 * Internal functions.
 */
void	fatal();
char	*syserr();

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
	auto struct termio	termio;

	if (myname = strrchr(av[0], '/'))
		++myname;
	else
		myname = av[0];
	if (ioctl(STDIN, TCGETA, &termio) < 0)
		fatal("ioctl(TCGETA)", syserr());
	termio.c_cc[VERASE] = ctl('h');
	termio.c_lflag |= ECHOE;
	if (ioctl(STDIN, TCSETA, &termio) < 0)
		fatal("ioctl(TCSETA)", syserr());
	exit(0);
	/* NOTREACHED */
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

	(void) write(STDERR, myname, (unsigned int) strlen(myname));
	(void) write(STDERR, between, (unsigned int) strlen(between));
	(void) write(STDERR, what, (unsigned int) strlen(what));
	(void) write(STDERR, between, (unsigned int) strlen(between));
	(void) write(STDERR, why, (unsigned int) strlen(why));
	(void) write(STDERR, after, (unsigned int) strlen(after));
	exit(1);
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
