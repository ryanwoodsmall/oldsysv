/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/ports.c	1.3"

#include "termio.h"
#include "dial.h"
#include "unistd.h"

#include "lpsched.h"

static void		sigalrm();

static int		SigAlrm;

/**
 ** open_dialup() - OPEN A PORT TO A ``DIAL-UP'' PRINTER
 **/

int			open_dialup (pp)
	register PRINTER	*pp;
{
	static char		*baud_table[]	= {
		      0,
		   "50",
		   "75",
		  "110",
		  "134",
		  "150",
		  "200",
		  "300",
		  "600",
		 "1200",
		 "1800",
		 "2400",
		 "4800",
		 "9600",
		"19200",
		"38400"
	};

	struct termio		tio;

	CALL			call;

	int			speed,
				fd;

	char			*sspeed;


	if ((speed = atoi(pp->speed)) <= 0)
		speed = -1;

	call.attr = 0;
	call.speed = speed;
	call.line = 0;
	call.telno = pp->dial_info;

	if ((fd = dial(call)) < 0)
		return (EXEC_EXIT_NDIAL | (~EXEC_EXIT_NMASK & abs(fd)));

	/*
	 * "dial()" doesn't guarantee which file descriptor
	 * it uses when it opens the port, so we probably have to
	 * move it.
	 */
	if (fd != 1) {
		dup2 (fd, 1);
		Close (fd);
	}

	/*
	 * The "printermgmt()" routines move out of ".stty"
	 * anything that looks like a baud rate, and puts it
	 * in ".speed", if the printer port is dialed. Thus
	 * we are saved the task of cleaning out spurious
	 * baud rates from ".stty".
	 *
	 * However, we must determine the baud rate and
	 * concatenate it onto ".stty" so that that we can
	 * override the default in the interface progam.
	 * Putting the override in ".stty" allows the user
	 * to override us (although it would be probably be
	 * silly for him or her to do so.)
	 */
	ioctl (1, TCGETA, &tio);
	if ((sspeed = baud_table[(tio.c_cflag & CBAUD)])) {

		register char	*new_stty = malloc(
			strlen(pp->stty) + 1 + strlen(sspeed) + 1
		);

		if (!new_stty) {
			errno = ENOMEM;
			return (EXEC_EXIT_NOMEM);
		}
		sprintf (new_stty, "%s %s", pp->stty, sspeed);

		/*
		 * We can trash "pp->stty" because
		 * the parent process has the good copy.
		 */
		pp->stty = new_stty;
	}

	return (0);
}

/**
 ** open_direct() - OPEN A PORT TO A DIRECTLY CONNECTED PRINTER
 **/

int			open_direct (pp)
	PRINTER			*pp;
{
	short			bufsz	    = -1,
				cps	    = -1;

	int			open_mode;

	register unsigned int	oldalarm,
				newalarm    = 0;

	register void		(*oldsig)() = signal(SIGALRM, sigalrm);


	/*
	 * Set an alarm to wake us from trying to open the port.
	 * We'll try at least 60 seconds, or more if the printer
	 * has a huge buffer that, in the worst case, would take
	 * a long time to drain.
	 */
	tidbit (pp->printer_type, "bufsz", &bufsz);
	tidbit (pp->printer_type, "cps", &cps);
	if (bufsz > 0 && cps > 0)
		newalarm = (((long)bufsz * 1100) / cps) / 1000;
	if (newalarm < 60)
		newalarm = 60;
	oldalarm = alarm(newalarm);

	/*
	 * The following open must be interruptable.
	 * O_APPEND is set in case the ``port'' is a file.
	 * O_RDWR is set in case the interface program wants
	 * to get input from the printer. Don't fail, though,
	 * just because we can't get read access.
	 */

	open_mode = O_WRONLY;
	if (access(pp->device, R_OK) == 0)
		open_mode = O_RDWR;
	open_mode |= O_APPEND;

	SigAlrm = 0;
	while (open(pp->device, open_mode, 0) == -1)
		if (errno != EINTR)
			return (EXEC_EXIT_NPORT);
		else if (SigAlrm)
			return (EXEC_EXIT_TMOUT);

	alarm (oldalarm);
	signal (SIGALRM, oldsig);

	return (0);
}

/**
 ** sigalrm()
 **/

static void		sigalrm ()
{
	signal (SIGALRM, SIG_IGN);
	SigAlrm = 1;
	return;
}

