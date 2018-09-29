/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/disp4.c	1.13"

#include "dispatch.h"

extern long		time();

/**
 ** accept_dest() - S_ACCEPT_DEST
 **/

void
accept_dest (m, n)
	char			*m;
	NODE			*n;
{
	char			*destination;

	ushort			status;

	register PSTATUS	*pps;

	register CSTATUS	*pcs;


	getmessage (m, S_ACCEPT_DEST, &destination);

	/*
	 * Have we seen this destination as a printer?
	 */
	if ((pps = search_ptable(destination)))
		if ((pps->status & PS_REJECTED) == 0)
			status = MERRDEST;
		else {
			pps->status &= ~PS_REJECTED;
			time (&pps->rej_date);
			dump_pstatus ();
			status = MOK;
		}

	/*
	 * Have we seen this destination as a class?
	 */
	else if ((pcs = search_ctable(destination)))
		if ((pcs->status & CS_REJECTED) == 0)
			status = MERRDEST;
		else {
			pcs->status &= ~CS_REJECTED;
			time (&pcs->rej_date);
			dump_cstatus ();
			status = MOK;
		}

	else
		status = MNODEST;

	send (n, R_ACCEPT_DEST, status);
	return;
}

/**
 ** reject_dest() - S_REJECT_DEST
 **/

void
reject_dest (m, n)
	char			*m;
	NODE			*n;
{
	char			*destination,
				*reason;

	ushort			status;

	register PSTATUS	*pps;

	register CSTATUS	*pcs;


	getmessage (m, S_REJECT_DEST, &destination, &reason);

	/*
	 * Have we seen this destination as a printer?
	 */
	if ((pps = search_ptable(destination)))
		if (pps->status & PS_REJECTED)
			status = MERRDEST;
		else {
			pps->status |= PS_REJECTED;
			time (&pps->rej_date);
			load_str (&pps->rej_reason, reason);
			dump_pstatus ();
			status = MOK;
		}

	/*
	 * Have we seen this destination as a class?
	 */
	else if ((pcs = search_ctable(destination)))
		if (pcs->status & CS_REJECTED)
			status = MERRDEST;
		else {
			pcs->status |= CS_REJECTED;
			time (&pcs->rej_date);
			load_str (&pcs->rej_reason, reason);
			dump_cstatus ();
			status = MOK;
		}

	else
		status = MNODEST;

	send (n, R_REJECT_DEST, status);
	return;
}

/**
 ** enable_dest() - S_ENABLE_DEST
 **/

void
enable_dest (m, n)
	char			*m;
	NODE			*n;
{
	char			*printer;

	ushort			status;

	register PSTATUS	*pps;


	getmessage (m, S_ENABLE_DEST, &printer);

	/*
	 * Have we seen this printer before?
	 */
	if ((pps = search_ptable(printer)))
		if (enable(pps) == -1)
			status = MERRDEST;
		else
			status = MOK;
	else
		status = MNODEST;

	send (n, R_ENABLE_DEST, status);
	return;
}

/**
 ** disable_dest() - S_DISABLE_DEST
 **/

void
disable_dest (m, n)
	char			*m;
	NODE			*n;
{
	char			*destination,
				*reason,
				*req_id		= 0;

	ushort			when,
				status;

	register PSTATUS	*pps;



	getmessage (m, S_DISABLE_DEST, &destination, &reason, &when);

	/*
	 * Have we seen this printer before?
	 */
	if ((pps = search_ptable(destination))) {

		/*
		 * If we are to cancel a currently printing request,
		 * we will send back the request's ID.
		 * Save a copy of the ID before calling "disable()",
		 * in case the disabling loses it (e.g. the request
		 * might get attached to another printer). (Actually,
		 * the current implementation won't DETACH the request
		 * from this printer until the child process responds,
		 * but a future implementation might.)
		 */
		if (pps->request && when == 2)
			req_id = strdup(pps->request->secure->req_id);

		if (disable(pps, reason, (int)when) == -1) {
			if (req_id) {
				free (req_id);
				req_id = 0;
			}
			status = MERRDEST;
		} else
			status = MOK;

	} else
		status = MNODEST;

	send (n, R_DISABLE_DEST, status, NB(req_id));
	if (req_id)
		free (req_id);

	return;
}

/**
 ** load_filter_table() - S_LOAD_FILTER_TABLE
 **/

void
load_filter_table (m, n)
	char			*m;
	NODE			*n;
{
	ushort			status;


	trash_filters ();
	if (Loadfilters((char *)0) == -1)
		status = MNOOPEN;

	else {

		/*
		 * This is what makes changing filters expensive!
		 */
		queue_check (qchk_filter);

		status = MOK;
	}

	send (n, R_LOAD_FILTER_TABLE, status);
	return;
}

/**
 ** unload_filter_table() - S_UNLOAD_FILTER_TABLE
 **/

void
unload_filter_table (m, n)
	char			*m;
	NODE			*n;
{
	trash_filters ();

	/*
	 * This is what makes changing filters expensive!
	 */
	queue_check (qchk_filter);

	send (n, R_UNLOAD_FILTER_TABLE, MOK);
	return;
}

/**
 ** load_user_file() - S_LOAD_USER_FILE
 **/

void
load_user_file (m, n)
	char			*m;
	NODE			*n;
{
	/*
	 * The first call to "getuser()" will load the whole file.
	 */
	trashusers ();

	send (n, R_LOAD_USER_FILE, MOK);
	return;
}

/**
 ** unload_user_file() - S_UNLOAD_USER_FILE
 **/

void
unload_user_file (m, n)
	char			*m;
	NODE			*n;
{
	trashusers ();	/* THIS WON'T DO TRUE UNLOAD, SORRY! */

	send (n, R_UNLOAD_USER_FILE, MOK);
	return;
}

/**
 ** shutdown() - S_SHUTDOWN
 **/

void
shutdown (m, n)
	char			*m;
	NODE			*n;
{
	ushort			immediate;

	(void)getmessage (m, S_SHUTDOWN, &immediate);
	send (n, R_SHUTDOWN, MOK);
	lpshut ();
	/*NOTREACHED*/
}

/**
 ** quiet() - S_QUIET
 **/

void
quiet (m, n)
	char			*m;
	NODE			*n;
{
	char			*name;

	ushort			type,
				status;

	register FSTATUS	*pfs;

	register PSTATUS	*pps;

	register PWSTATUS	*ppws;


#if defined(TRACE_MALLOC)
	dump_malloc (stdout);
	dump_arena (stdout);
	fflush(stdout);
#endif
#if	defined(MDL)
	mdldump(0);
#endif

	/*
	 * We quiet an alert by cancelling it with "cancel_alert()"
	 * and then resetting the active flag. This effectively just
	 * terminates the process running the alert but tricks the
	 * rest of the Spooler into thinking it is still active.
	 * The alert will be reactivated only AFTER "cancel_alert()"
	 * has been called (to clear the active flag) and then "alert()"
	 * is called again. Thus:
	 *
	 * For printer faults the alert will be reactivated when:
	 *	- a fault is found after the current fault has been
	 *	  cleared (i.e. after successful print or after manually
	 *	  enabled).
	 *
	 * For forms/print-wheels the alert will be reactivated when:
	 *	- the form/print-wheel becomes mounted and then unmounted
	 *	  again, with too many requests still pending;
	 *	- the number of requests falls below the threshold and
	 *	  then rises above it again.
	 */

	(void)getmessage (m, S_QUIET_ALERT, &name, &type);

	if (!*name)
		status = MNODEST;

	else switch (type) {
	case QA_FORM:
		if (!(pfs = search_ftable(name)))
			status = MNODEST;

		else if (!pfs->alert->active)
			status = MERRDEST;

		else {
			cancel_alert (A_FORM, pfs);
			pfs->alert->active = 1;
			status = MOK;
		}
		break;
		
	case QA_PRINTER:
		if (!(pps = search_ptable(name)))
		    status = MNODEST;

		else if (!pps->alert->active)
			status = MERRDEST;

		else {
			cancel_alert (A_PRINTER, pps);
			pps->alert->active = 1;
			status = MOK;
		}
		break;
		
	case QA_PRINTWHEEL:
		if (!(ppws = search_pwtable(name)))
		    status = MNODEST;

		else if (!ppws->alert->active)
			status = MERRDEST;

		else {
			cancel_alert (A_PWHEEL, ppws);
			ppws->alert->active = 1;
			status = MOK;
		}
		break;
		
	}
	
	send (n, R_QUIET_ALERT, status);
	return;
}

/**
 ** send_fault() - S_SEND_FAULT
 **/

void
send_fault (m, n)
	char			*m;
	NODE			*n;
{
	long			key;

	char			*printer,
				*alert_text;

	ushort			status;

	register PSTATUS	*pps;


	getmessage (m, S_SEND_FAULT, &printer, &key, &alert_text);

	if (
		!(pps = search_ptable(printer))
	     || !pps->exec
	     || pps->exec->key != key
	     || !pps->request
	)
		status = MERRDEST;

	else {
		printer_fault (pps, pps->request, alert_text, 0);
		status = MOK;

	}

	send (n, R_SEND_FAULT, status);
	return;
}
