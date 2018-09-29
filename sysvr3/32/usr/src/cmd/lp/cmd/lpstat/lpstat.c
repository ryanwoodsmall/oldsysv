/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/lpstat.c	1.5"

#include "stdio.h"
#include "errno.h"
#include "sys/types.h"
#include "signal.h"

#include "lp.h"
#include "msgs.h"
#include "printers.h"

#include "lpstat.h"


#ifdef SIGPOLL
void			catch();
#else
int			catch();
#endif

int			exit_rc			= 0,
			inquire_type		= INQ_UNKNOWN,
			scheduler_active	= 0,
			r;		/* Says -r was specified */

char			*alllist[]	= {
	NAME_ALL,
	0
};

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	parse (argc, argv);
	done (0);
	/*NOTREACHED*/
}

/**
 ** def()
 **/

void			def ()
{
	char			*name;

	if ((name = getdefault()))
		PRINTF ("system default destination: %s\n", name);
	else
		PRINTF ("no system default destination\n");

	return;
}

/**
 ** running()
 **/

void			running ()
{
	PRINTF (
		"scheduler is %s\n",
		(scheduler_active ? "running" : "not running")
	);
	return;
}

/**
 ** startup()
 **/

void			startup ()
{
	int			try;


	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void)signal (SIGHUP, catch);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void)signal (SIGINT, catch);

	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		(void)signal (SIGQUIT, catch);

	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		(void)signal (SIGTERM, catch);

	for (try = 1; try <= 5; try++) {
		scheduler_active = (mopen() == 0);
		if (scheduler_active || errno != ENOSPC)
			break;
		sleep (3);
	}

	return;
}

/**
 ** catch()
 **/

#ifdef SIGPOLL
void
#else
int
#endif
			catch()
{
	(void)signal (SIGHUP, SIG_IGN);
	(void)signal (SIGINT, SIG_IGN);
	(void)signal (SIGQUIT, SIG_IGN);
	(void)signal (SIGTERM, SIG_IGN);
	done (2);
}
