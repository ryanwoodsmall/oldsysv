/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:model/lp.cat.c	1.6"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "termio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
#include "sys/times.h"
#include "string.h"

#include "lp.h"

#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)

#define E_SUCCESS	0
#define E_BAD_INPUT	1
#define E_BAD_OUTPUT	2
#define E_BAD_TERM	3
#define E_IDENTICAL	4
#define	E_WRITE_FAILED	5
#define	E_TIMEOUT	6
#define E_HANGUP	7
#define E_INTERRUPT	8

#define SAFETY_FACTOR	2.0
#define R(F)		(int)((F) + .5)
#define DELAY(N,D)	R(SAFETY_FACTOR * ((N) / (double)(D)))

extern int		sys_nerr;

extern char		*sys_errlist[],
			*getenv();

extern int		atoi();

char			buffer[BUFSIZ];

void			sighup(),
			sigint(),
			sigquit(),
			sigpipe(),
			sigalrm(),
			sigterm();

#if	defined(baudrate)
# undef	baudrate
#endif

int			baudrate();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	register int		nin,
				nout,
				effective_rate,
				max_delay	= 0,
				n;

	int			report_rate;

	short			print_rate;

	struct stat		in,
				out;

	struct tms		tms;

	long			epoch_start,
				epoch_end;

	char			*TERM;


	if (argc > 1 && STREQU(argv[1], "-r")) {
		report_rate = 1;
		argc--;
		argv++;
	} else
		report_rate = 0;

	/*
	 * Stat the standard output to be sure it is defined.
	 */
	if (fstat(1, &out) < 0) {
		fprintf (
			stderr,
		"Can't stat output (%s);\nincorrect use of lp.cat!\n",
			PERROR
		);
		exit (E_BAD_OUTPUT);
	}

	/*
	 * Stat the standard input to be sure it is defined.
	 */
	if (fstat(0, &in) < 0) {
		fprintf (
			stderr,
		"Can't stat input (%s);\nincorrect use of lp.cat!\n",
			PERROR
		);
		exit (E_BAD_INPUT);
	}

	/*
	 * If the standard output is not a character special file or a
	 * block special file, make sure it is not identical to the
	 * standard input.
	 */
	if (!ISCHR(out) && !ISBLK(out) && IDENTICAL(out, in)) {
		fprintf (
			stderr,
	"Input and output are identical; incorrect use of lp.cat!\n"
		);
		exit (E_IDENTICAL);
	}

	/*
	 * The effective data transfer rate is the lesser
	 * of the transmission rate and print rate. If an
	 * argument was passed to us, it should be a data
	 * rate and it may be lower still.
	 * Based on the effective data transfer rate,
	 * we can predict the maximum delay we should experience.
	 * But there are other factors that could introduce
	 * delay, so let's be generous; after all, we'd rather
	 * err in favor of waiting too long to detect a fault
	 * than err too often on false alarms.
	 */

	if (
		!(TERM = getenv("TERM"))
	     || !*TERM
	) {
		fprintf (
			stderr,
	"No TERM variable defined! Trouble with the Spooler!\n"
		);
		exit (E_BAD_TERM);
	}
	if (
		!STREQU(TERM, NAME_UNKNOWN)
	     && tidbit(TERM, "cps", &print_rate) == -1
	) {
		fprintf (
			stderr,
"Trouble identifying printer type \"%s\"; check the Terminfo database.\n",
			TERM
		);
		exit (E_BAD_TERM);
	}
	if (STREQU(TERM, NAME_UNKNOWN))
		print_rate = -1;

	effective_rate = baudrate() / 10; /* okay for most bauds */
	if (print_rate != -1 && print_rate < effective_rate)
		effective_rate = print_rate;
	if (argc > 1 && (n = atoi(argv[1])) >= 0 && n < effective_rate)
		effective_rate = n;	  /* 0 means infinite delay */
	if (effective_rate)
		max_delay = DELAY(BUFSIZ, effective_rate);

	/*
	 * We'll use the "alarm()" system call to keep us from
	 * waiting too long to write to a printer in trouble.
	 * Other faults:
	 *	- hangup (drop of carrier)
	 *	- interrupt (printer sent a break or quit character)
	 *	- SIGPIPE (output port is a FIFO, and was closed early)
	 *	- terminated (not a fault, but we must catch it)
	 */
	if (max_delay)
		signal (SIGALRM, sigalrm);
	signal (SIGHUP, sighup);
	signal (SIGINT, sigint);
	signal (SIGQUIT, sigint);
	signal (SIGPIPE, sigpipe);
	signal (SIGTERM, sigterm);

	/*
	 * While not end of standard input, copy blocks to
	 * standard output.
	 */
	while ((nin = read(0, buffer, BUFSIZ)) > 0) {

		if (max_delay)
			alarm (max_delay);
		if (report_rate)
			epoch_start = times(&tms);

		if ((nout = write(1, buffer, nin)) != nin) {
			if (nout < 0)
				fprintf (
					stderr,
	"Write failed (%s);\nperhaps the printer has gone off-line.\n",
					PERROR
				);
			else
				fprintf (
					stderr,
	"Incomplete write; perhaps the printer has gone off-line.\n"
				);
			exit (E_WRITE_FAILED);
		}
		if (max_delay)
			alarm (0);
		else if (report_rate) {
			epoch_end = times(&tms);
			if (epoch_end > epoch_start)
				fprintf (
					stderr,
					"%d CPS\n",
		R((100 * BUFSIZ) / (double)(epoch_end - epoch_start))
				);
		}

	}

	exit (E_SUCCESS);
	/*NOTREACHED*/
}

/**
 ** sighup() - CATCH A HANGUP (LOSS OF CARRIER)
 **/

void			sighup ()
{
	char			*msg = HANGUP_FAULT;

	msg[strlen(msg) - 2] = '.';
	signal (SIGHUP, SIG_IGN);
	fprintf (stderr, msg);
	exit (E_HANGUP);
}

/**
 ** sigint() - CATCH AN INTERRUPT
 **/

void			sigint ()
{
	signal (SIGINT, SIG_IGN);
	fprintf (stderr, INTERRUPT_FAULT);
	exit (E_INTERRUPT);
}

/**
 ** sigpipe() - CATCH EARLY CLOSE OF PIPE
 **/

void			sigpipe ()
{
	signal (SIGPIPE, SIG_IGN);
	fprintf (stderr, PIPE_FAULT);
	exit (E_INTERRUPT);
}

/**
 ** sigalrm() - CATCH AN ALARM
 **/

void			sigalrm ()
{
	fprintf (
		stderr,
	"Excessive write delay; perhaps the printer has gone off-line.\n"
	);
	exit (E_TIMEOUT);
}

/**
 ** sigterm() - CATCH A TERMINATION SIGNAL
 **/

void			sigterm ()
{
	signal (SIGTERM, SIG_IGN);
	exit (E_SUCCESS);
}

/**
 ** baudrate() - RETURN BAUD RATE OF OUTPUT LINE
 **/

static int		baud_convert[] =
{
	0, 50, 75, 110, 135, 150, 200, 300, 600, 1200,
	1800, 2400, 4800, 9600, 19200, 38400
};

int			baudrate ()
{
	struct termio		tm;

	if (ioctl(1, TCGETA, &tm) < 0)
		return (1200);
	return (tm.c_cflag&CBAUD ? baud_convert[tm.c_cflag&CBAUD] : 1200);
}
