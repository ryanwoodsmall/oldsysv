/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/daisyforms.c	1.5"

#include "lpsched.h"

/**
 ** queue_form() - ADD A REQUEST TO A FORM QUEUE
 **/

void			queue_form (prs, pfs)
	register RSTATUS	*prs;
	FSTATUS			*pfs;
{
	if ((prs->form = pfs)) {
		prs->form->requests++;
		check_form_alert (prs->form, (_FORM *)0);
	}
	return;
}

/**
 ** unqueue_form() - REMOVE A REQUEST FROM A FORM QUEUE
 **/

void			unqueue_form (prs)
	register RSTATUS	*prs;
{
	if (prs->form) {
		prs->form->requests--;
		check_form_alert (prs->form, (_FORM *)0);
		prs->form = 0;
	}
	return;
}

/**
 ** queue_pwheel() - ADD A REQUEST TO A PRINT WHEEL QUEUE
 **/

void			queue_pwheel (prs, name)
	register RSTATUS	*prs;
	char			*name;
{
	if (name) {
		prs->pwheel_name = strdup(name);
		/*
		 * Don't bother queueing the request for
		 * a print wheel if this request is destined for
		 * only this printer and the printer doesn't take
		 * print wheels.
		 */
		if (
			!one_printer_with_charsets(prs)
		     && (prs->pwheel = search_pwtable(name))
		) {
			prs->pwheel->requests++;
			check_pwheel_alert (prs->pwheel, (PWHEEL *)0);
		}
	}
	return;
}

/**
 ** unqueue_pwheel() - REMOVE A REQUEST FROM A PRINT WHEEL QUEUE
 **/

void			unqueue_pwheel (prs)
	register RSTATUS	*prs;
{
	if (prs->pwheel) {
		prs->pwheel->requests--;
		check_pwheel_alert (prs->pwheel, (PWHEEL *)0);
		prs->pwheel = 0;
	}
	unload_str (&(prs->pwheel_name));
	return;
}

/**
 ** check_form_alert() - CHECK CHANGES TO MOUNT FORM ALERT
 **/

void			check_form_alert (pfs, pf)
	register FSTATUS	*pfs;
	register _FORM		*pf;
{
	register int		trigger,
				fire_off_alert	= 0;

	/*
	 * Call this routine whenever a requests has been queued
	 * or dequeued for a form, and whenever the form changes.
	 * If a pointer to a new _FORM is passed, the FSTATUS
	 * structure is updated with the changes. Use a second
	 * argument of 0 if no change.
	 *
	 * WARNING: It is valid to call this routine when adding
	 * a NEW form (not just changing it). Thus the members of
	 * the structure "pfs->form" may not be set.
	 * In this case, though, "pf" MUST be set, and there can
	 * be NO alert active.
	 */

	if (pf) {
		if ((trigger = pf->alert.Q) <= 0)
			trigger = 1;
	} else
		trigger = pfs->trigger;

	if (Starting || pfs->mounted)
		goto Return;

#define	OALERT	pfs->form->alert
#define NALERT	pf->alert

	/*
	 * Cancel an active alert if the number of requests queued
	 * has dropped below the threshold (or the threshold has been
	 * raised), or if the alert command or period has changed.
	 * In the latter case we'll reactive the alert later.
	 */
	if (pfs->alert->active)
		if (!pfs->requests || pfs->requests < trigger)
			cancel_alert (A_FORM, pfs);

		else if (
			pf
		     && (
				!SAME(NALERT.shcmd, OALERT.shcmd)
			     || NALERT.W != OALERT.W
			     || NALERT.Q != OALERT.Q
			)
		)
			cancel_alert (A_FORM, pfs);

	/*
	 * If we still have the condition for an alert, we'll fire
	 * one off. It is possible the alert is still running, but
	 * that's okay. First, we may want to change the alert message;
	 * second, the "alert()" routine doesn't execute an alert
	 * if it is already running.
	 */
	if (trigger > 0 && pfs->requests >= trigger)
		if ((pf && NALERT.shcmd) || OALERT.shcmd)
			fire_off_alert = 1;

#undef	OALERT
#undef	NALERT

Return:	if (pf) {
		/*
		 * Watch it! We may be adding a new form, so there
		 * may be nothing to toss out.
		 */
		if (pfs->form->name)
			freeform (pfs->form);

		*(pfs->form) = *pf;
		pfs->trigger = trigger;
	}

	/*
	 * Have to do this after updating the changes.
	 */
	if (fire_off_alert)
		alert (A_FORM, pfs);

	return;
}

/**
 ** check_pwheel_alert() - CHECK CHANGES TO MOUNT PRINTWHEEL ALERT
 **/

void			check_pwheel_alert (ppws, ppw)
	register PWSTATUS	*ppws;
	register PWHEEL		*ppw;
{
	register int		trigger,
				fire_off_alert	= 0;

	/*
	 * Call this routine whenever a request has been queued
	 * or dequeued for a print-wheel, and whenever the print-wheel
	 * changes. If a pointer to a new PWHEEL is passed, the
	 * PWSTATUS structure is updated with the changes. Use a
	 * second argument of 0 if no change.
	 *
	 * WARNING: It is valid to call this routine when adding
	 * a NEW print wheel (not just changing it). Thus the members
	 * of the structure "ppws->pwheel" may not be set.
	 * In this case, though, "ppw" MUST be set, and there can
	 * be NO alert active.
	 */

	if (ppw) {
		if ((trigger = ppw->alert.Q) <= 0)
			trigger = 1;
	} else
		trigger = ppws->trigger;

	if (Starting || ppws->mounted)
		goto Return;

#define	OALERT	ppws->pwheel->alert
#define NALERT	ppw->alert

	/*
	 * Cancel an active alert if the number of requests queued
	 * has dropped below the threshold (or the threshold has been
	 * raised), or if the alert command or period has changed.
	 * In the latter case we'll reactive the alert later.
	 */
	if (ppws->alert->active)
		if (!ppws->requests || ppws->requests < trigger)
			cancel_alert (A_PWHEEL, ppws);

		else if (
			ppw
		     && (
				!SAME(NALERT.shcmd, OALERT.shcmd)
			     || NALERT.W != OALERT.W
			     || NALERT.Q != OALERT.Q
			)
		)
			cancel_alert (A_PWHEEL, ppws);

	/*
	 * If we still have the condition for an alert, we'll fire
	 * one off. It is possible the alert is still running, but
	 * that's okay. First, we may want to change the alert message;
	 * second, the "alert()" routine doesn't execute an alert
	 * if it is already running.
	 */
	if (trigger > 0 && ppws->requests && ppws->requests >= trigger)
		if ((ppw && NALERT.shcmd) || OALERT.shcmd)
			fire_off_alert = 1;

#undef	OALERT
#undef	NALERT

Return:	if (ppw) {
		/*
		 * Watch it! We may be adding a new print wheel, so there
		 * may be nothing to toss out.
		 */
		if (ppws->pwheel->name)
			freepwheel (ppws->pwheel);

		*(ppws->pwheel) = *ppw;
		ppws->trigger = trigger;
	}

	/*
	 * Have to do this after updating the changes.
	 */
	if (fire_off_alert)
		alert (A_PWHEEL, ppws);

	return;
}

/**
 ** one_printer_with_charsets() 
 **/

int			one_printer_with_charsets (prs)
	register RSTATUS	*prs;
{
	/*
	 * This little function answers the question: Is a request
	 * that needs a character set destined for a particular
	 * printer that has selectable character sets instead of
	 * mountable print wheels?
	 */
	return (
	    STREQU(prs->request->destination, prs->printer->printer->name)
	 && !prs->printer->printer->daisy
	);
}
