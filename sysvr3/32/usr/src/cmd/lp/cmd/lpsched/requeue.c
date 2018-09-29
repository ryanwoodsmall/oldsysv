/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/requeue.c	1.6"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lpsched.h"
#include "validate.h"

/*
 * The routines in this file are used to examine queued requests
 * to see if something must be done about them. We don't bother
 * checking requests that are:
 *
 *	- printing (we could, to allow the administrator to stop
 *	  a request by making a configuration change, but that
 *	  can lead to trouble (yet another way to terminate a child)
 *	  and the administrator can always disable the request to
 *	  force it to stop printing and be reevaluated);
 *
 *	- changing, since once the change is complete the request
 *	  will be reevaluated again anyway;
 *
 *	- notifying, since the request is essentially finished
 *
 *	- done.
 *
 * Requests that are being held or are filtering ARE to be considered,
 * because things may have changed to make them impossible to print.
 */
#define	SKIP_IT(PRS) \
((PRS)->request->outcome & (RS_PRINTING|RS_CHANGING|RS_NOTIFYING|RS_DONE))

/**
 ** queue_attract() - REASSIGN REQUESTS TO PRINTER, IF POSSIBLE
 **/

void			queue_attract (pps, qchk_p)
	register PSTATUS	*pps;
	register int		(*qchk_p)();
{
	register RSTATUS	*prs;

	register CSTATUS	*pcs;

#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
	register int		inclass;
#endif


	/*
	 * Evaluate requests that:
	 *	- meet a criteria set by a function passed.
	 *	- are already queued for the printer
	 *	- are destined for a class containing this printer
	 *	- or are destined for any printer
	 * We stop on the first one that will work on the printer,
	 * and schedule an interface for the printer (which will
	 * find the first request ready, namely the one we stopped on).
	 */

#define	SAMECLASS(PRS,PPS) \
	( \
		(pcs = search_ctable(PRS->request->destination)) \
	     && searchlist(PPS->printer->name, pcs->class->members) \
	)

#define ISANY(PRS)	STREQU(PRS->request->destination, NAME_ANY)

	for (prs = Request_List; prs; prs = prs->next) {
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
		inclass = 0;
#endif
		if (
			!SKIP_IT(prs)
		     && (!qchk_p || (*qchk_p)(prs))
		     && (
				prs->printer == pps
			     || ISANY(prs)
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
			     || (inclass = SAMECLASS(prs, pps))
#else
			     || SAMECLASS(prs, pps)
#endif
			)
		)
			/*
			 * Don't need to evaluate the request if it
			 * is already queued!
			 */
			if (
				prs->printer == pps
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
			     || evaluate_request(prs, pps, inclass) == MOK
#else
			     || evaluate_request(prs, pps, 0) == MOK
#endif
			) {
				/*
				 * This request was attracted to the
				 * printer but maybe it now needs to be
				 * filtered. If so, filter it but see if
				 * there's another request all set to go.
				 */
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else {
					schedule (EV_INTERF, pps);
					break;
				}
			}
	}

	return;
}

/**
 ** queue_repel() - REASSIGN REQUESTS TO ANOTHER PRINTER, IF POSSIBLE
 **/

int			queue_repel (pps, move_off, qchk_p)
	register PSTATUS	*pps;
	register int		move_off,
				(*qchk_p)();
{
	register RSTATUS	*prs;

	register int		all_can		= 1;

	register PSTATUS	*stop_pps	= (move_off? pps : 0);


	/*
	 * Reevaluate all requests that are assigned to this
	 * printer, to see if there's another printer that
	 * can handle them.
	 *
	 * If the "move_off" flag is set, don't consider the current
	 * printer when reevaluating, but also don't cancel the request
	 * if it can't be moved off the printer.
	 * (Currently this is only used when deciding if a printer
	 * can be deleted.)
	 */
	BEGIN_WALK_BY_PRINTER_LOOP (prs, pps)
		if (!SKIP_IT(prs) && (!qchk_p || (*qchk_p)(prs)))

			if (reevaluate_request(prs, stop_pps) == MOK) {

				/*
				 * If this request needs to be filtered,
				 * try to schedule it for filtering,
				 * otherwise schedule it for printing.
				 * We are inefficient here, because we may
				 * try to schedule many requests but the
				 * filtering slot(s) and printers are
				 * busy; but the requests may languish
				 * if we don't check here.
				 */
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else
					schedule (EV_INTERF, prs->printer);

			} else {
				all_can = 0;
				if (!move_off)
					cancel (prs, 1);
				else
					prs->reason = MOK;
			}
	END_WALK_LOOP

	return (all_can);
}

/**
 ** queue_check() - CHECK ALL REQUESTS AGAIN
 **/

void			queue_check (qchk_p)
	register int		(*qchk_p)();
{
	register RSTATUS	*prs;


	for (prs = Request_List; prs; prs = prs->next)
		if (!SKIP_IT(prs) && (!qchk_p || (*qchk_p)(prs)))
			if (reevaluate_request(prs, (PSTATUS *)0) == MOK)
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else
					schedule (EV_INTERF, prs->printer);
			else
				cancel (prs, 1);

	return;
}

/**
 ** qchk_waiting() - CHECK IF REQUEST IS READY TO PRINT
 ** qchk_filter() - CHECK IF REQUEST NEEDS A FILTER
 ** qchk_form() - CHECK IF REQUEST NEEDS A FORM
 ** qchk_pwheel() - CHECK IF REQUEST NEEDS PRINT A WHEEL
 **/

int			qchk_waiting (prs)
	RSTATUS						*prs;
{
	return (
		!(prs->request->outcome & (RS_HELD|RS_DONE|RS_ACTIVE))
	     && !NEEDS_FILTERING(prs)
	);
}

int			qchk_filter (prs)
	register RSTATUS	*prs;
{
	/*
	 * No need to reevaluate this request if it isn't using a filter
	 * or if it is done or is being changed.
	 */
	return (
		!(prs->request->outcome & (RS_DONE|RS_CHANGING|RS_NOTIFY))
	     && (prs->slow || prs->fast)
	);
}

FSTATUS			*form_in_question;

int			qchk_form (prs)
	register RSTATUS	*prs;
{
	return (prs->form == form_in_question);
}

char			*pwheel_in_question;

int			qchk_pwheel (prs)
	register RSTATUS	*prs;
{
	return (SAME(prs->pwheel_name, pwheel_in_question));
}
