/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/disena.c	1.9"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lpsched.h"

extern long		time();

/**
 ** disable() - DISABLE PRINTER
 **/

int
disable (pps, reason, when)
	register PSTATUS	*pps;
	char			*reason;
	int			when;
{
	if (pps->status & PS_DISABLED)
		return (-1);

	else {
		pps->status |= PS_DISABLED;
		time (&pps->dis_date);
		load_str (&pps->dis_reason, reason);

		dump_pstatus ();

		if (pps->status & PS_BUSY)
			switch (when) {

			case 0:		/* Stop current job, requeue. */
				if (pps->request)
				    pps->request->request->outcome |= RS_STOPPED;
				terminate (pps->exec);
				break;

			case 1:		/* Let current job finish. */
				break;

			case 2:		/* Cancel current job outright. */
				if (pps->request)
				    cancel (pps->request, 1);
				break;

			}

/*		(void)queue_repel (pps, 0, (int (*)())0);	*/
		return (0);
	}
}

/**
 ** enable() - ENABLE PRINTER
 **/

int
enable (pps)
	register PSTATUS	*pps;
{
	/*
	 * ``Enabling a printer'' includes clearing a fault and
	 * clearing the do-it-later flag to allow the printer
	 * to start up again.
	 */
	if (!(pps->status & (PS_FAULTED|PS_DISABLED|PS_LATER)))
		return (-1);

	else {
		pps->status &= ~(PS_FAULTED|PS_DISABLED|PS_LATER);
		(void) time (&pps->dis_date);

		dump_pstatus ();

		if (pps->alert->active)
			cancel_alert (A_PRINTER, pps);

		/*
		 * Attract the FIRST request that is waiting to
		 * print to this printer. In this regard we're acting
		 * like the printer just finished printing a request
		 * and is looking for another.
		 */
		queue_attract (pps, qchk_waiting);
		return (0);
	}
}
