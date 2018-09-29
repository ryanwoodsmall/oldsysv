/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:model/lp.page.c	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "signal.h"

#include "lp.h"

void			flush_page(),
			sighup(),
			sigint(),
			sigquit(),
			sigpipe(),
			sigterm();

int			lines,
			n;

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	char			buf[BUFSIZ];


	signal (SIGHUP, sighup);
	signal (SIGINT, sigint);
	signal (SIGQUIT, sigint);
	signal (SIGPIPE, sigpipe);
	signal (SIGTERM, sigterm);

	if (argc != 2)
		lines = 66;

	else if ((lines = atoi(argv[1])) < 1)
		lines = 66;

	n = 0;
	while (gets(buf)) {
		puts (buf);
		if (++n > lines)
			n = 1;
	}

	flush_page ();

	return (0);
}

/**
 ** flush_page()
 **/

void			flush_page ()
{
	while (n++ < lines)
		putchar ('\n');
	fflush (stdout);
	return;
}

/**
 ** sighup() - CATCH A HANGUP (LOSS OF CARRIER)
 **/

void			sighup ()
{
	signal (SIGHUP, SIG_IGN);
	fprintf (stderr, HANGUP_FAULT);
	exit (1);
}

/**
 ** sigint() - CATCH AN INTERRUPT
 **/

void			sigint ()
{
	signal (SIGINT, SIG_IGN);
	fprintf (stderr, INTERRUPT_FAULT);
	exit (1);
}

/**
 ** sigpipe() - CATCH EARLY CLOSE OF PIPE
 **/

void			sigpipe ()
{
	signal (SIGPIPE, SIG_IGN);
	fprintf (stderr, PIPE_FAULT);
	exit (1);
}

/**
 ** sigterm() - CATCH A TERMINATION SIGNAL AND FORCE FULL PAGE
 **/

void			sigterm ()
{
	signal (SIGTERM, SIG_IGN);
	flush_page ();
	exit (0);
}
