/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/cancel.c	1.7"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "signal.h"
#include "string.h"
#include "errno.h"
#include "varargs.h"
#include "sys/types.h"

#include "lp.h"
#include "requests.h"
#include "msgs.h"

#define WHO_AM_I	I_AM_CANCEL
#include "oam.h"

/*
 * There are no sections of code in this progam that have to be
 * protected from interrupts. We do want to catch them, however,
 * so we can clean up properly.
 */

extern char		*malloc();

void			startup(),
			cleanup(),
			cancel(),
			restart(),	/* a misnomer */
			send_message(),
			recv_message();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	int			i;

	char			*arg;

	if (argc == 1 || STREQU(argv[1], "-?")) {
		printf ("usage: cancel id ... printer ...\n");
		exit (0);
	}

	startup ();

	for (i = 1; i < argc; i++) {

		arg = argv[i];

		if (isrequest(arg))
			cancel (arg);
		else if (isprinter(arg))
			restart (arg);
		else
			LP_ERRMSG1 (WARNING, E_CAN_BADARG, arg);

	}

	cleanup ();
	return (0);
}

/**
 ** cancel() - CANCEL ONE REQUEST
 **/

void			cancel (req)
	char			*req;
{
	short			status;

	/*
	 * Now try to cancel the request.
	 */

	send_message (S_CANCEL_REQUEST, req);
	recv_message (R_CANCEL_REQUEST, &status);

	switch (status) {

	case MOK:
		printf ("request \"%s\" cancelled\n", req);
		break;

#if	defined(NETWORKING)
	case MOKREMOTE:
		LP_ERRMSG1 (INFO, E_CAN_REMOTE, req);
		break;
#endif

	case MUNKNOWN:
		LP_ERRMSG1 (WARNING, E_LP_UNKREQID, req);
		break;

	case M2LATE:
		LP_ERRMSG1 (WARNING, E_LP_2LATE, req);
		break;

	case MNOPERM:
		LP_ERRMSG1 (WARNING, E_CAN_CANT, req);
		break;

	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, status);
		cleanup ();
		exit (1);
	}
	return;
}

/**
 ** restart() - CANCEL REQUEST CURRENTLY PRINTING ON GIVEN PRINTER
 **/

void			restart (printer)
	char			*printer;
{
	char			*req_id,
				*s_ignore;

	short			status,
				h_ignore;

	long			l_ignore;


	/*
	 * Get the list of requests queued for this printer.
	 */

	send_message (S_INQUIRE_PRINTER_STATUS, printer);
	recv_message (
		R_INQUIRE_PRINTER_STATUS,
		&status,
		&s_ignore,	/* printer	*/
		&s_ignore,	/* form		*/
		&s_ignore,	/* print_wheel	*/
		&s_ignore,	/* dis_reason	*/
		&s_ignore,	/* rej_reason	*/
		&h_ignore,	/* p_status	*/
		&req_id,
		&l_ignore,	/* dis_date	*/
		&l_ignore	/* rej_date	*/
	);

	switch (status) {

	case MOK:
		if (!req_id || !*req_id)
			LP_ERRMSG1 (WARNING, E_LP_PNBUSY, printer);
		else
			cancel (req_id);
		break;

	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, status);
		cleanup ();
		exit (1);

	}

	return;
}

/**
 ** startup() - OPEN MESSAGE QUEUE TO SPOOLER
 **/

void			startup ()
{
	void			catch();

	/*
	 * Open a private queue for messages to the Spooler.
	 * An error is deadly.
	 */
	if (mopen() == -1) {

		switch (errno) {
		case ENOMEM:
		case ENOSPC:
			LP_ERRMSG (ERROR, E_LP_MLATER);
			break;
		default:
			LP_ERRMSG (ERROR, E_LP_NEEDSCHED);
			break;
		}

		exit (1);
	}

	/*
	 * Now that the queue is open, quickly trap signals
	 * that we might get so we'll be able to close the
	 * queue again, regardless of what happens.
	 */
	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, catch);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, catch);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, catch);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, catch);
}

/**
 ** catch() - CATCH INTERRUPT, HANGUP, ETC.
 **/

void			catch (sig)
	int			sig;
{
	signal (sig, SIG_IGN);
	cleanup ();
	exit (1);
}

/**
 ** cleanup() - CLOSE THE MESSAGE QUEUE TO THE SPOOLER
 **/

void			cleanup ()
{
	(void)mclose ();
	return;
}

/**
 ** send_message() - HANDLE MESSAGE SENDING TO SPOOLER
 **/

/*VARARGS0*/

void			send_message (va_alist)
	va_dcl
{
	va_list			ap;

	int			type;

	char			*p1,
				buffer[MSGMAX];


	va_start (ap);
	switch ((type = va_arg(ap, int))) {
	case S_INQUIRE_PRINTER_STATUS:
	case S_CANCEL_REQUEST:
		p1 = va_arg(ap, char *);
		(void)putmessage (buffer, type, p1);
		break;
	}
	va_end (ap);

	if (msend(buffer) == -1) {
		LP_ERRMSG (ERROR, E_LP_MSEND);
		cleanup ();
		exit (1);
	}

	return;
}

/**
 ** recv_message() - HANDLE MESSAGES BACK FROM SPOOLER
 **/

/*VARARGS0*/

void			recv_message (va_alist)
	va_dcl
{
	va_list			ap;

	static char		buffer[MSGMAX];

	int			rc,
				type;


	va_start (ap);

	type = va_arg(ap, int);
	if (mrecv(buffer, MSGMAX) != type) {
		LP_ERRMSG (ERROR, E_LP_MRECV);
		cleanup ();
		exit (1);
	}

	switch(type) {
	case R_INQUIRE_PRINTER_STATUS:
	case R_CANCEL_REQUEST:
		rc = _getmessage(buffer, type, ap);
		if (rc != type) {
			LP_ERRMSG1 (ERROR, E_LP_BADREPLY, rc);
			cleanup ();
			exit (1);
		}
		break;
	}

	va_end (ap);

	return;
}
