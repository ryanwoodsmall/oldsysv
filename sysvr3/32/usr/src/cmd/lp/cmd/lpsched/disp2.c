/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/disp2.c	1.9"

#include	"dispatch.h"

extern long		time();

/**
 ** load_printer() - S_LOAD_PRINTER
 **/

void
load_printer (m, n)
	char			*m;
	NODE			*n;
{
	char			*printer;

	ushort			status;

	PRINTER			op;

	register PRINTER	*pp;

	register PSTATUS	*pps;


	(void)getmessage (m, S_LOAD_PRINTER, &printer);

	if (!*printer)
		status = MNODEST;

	/*
	 * Strange or missing printer?
	 */
	else if (!(pp = Getprinter(printer))) {
		switch (errno) {
		case EBADF:
			status = MERRDEST;
			break;
		case ENOENT:
		default:
			status = MNODEST;
			break;
		}

	/*
	 * Printer we know about already?
	 */
	} else if ((pps = search_ptable(printer))) {

		op = *(pps->printer);
		*(pps->printer) = *pp;

		/*
		 * Ensure that an old Terminfo type that's no longer 
		 * needed gets freed, and that an existing type gets
		 * reloaded (in case it has been changed).
		 */
		untidbit (op.printer_type);
		untidbit (pp->printer_type);

		/*
		 * Does an alert get affected?
		 *	- Different command?
		 *	- Different wait interval?
		 */
		if (pps->alert->active)

#define	OALERT	op.fault_alert
#define NALERT	pp->fault_alert
			if (
				!SAME(NALERT.shcmd, OALERT.shcmd)
			     || NALERT.W != OALERT.W
			) {
				/*
				 * We can't use "cancel_alert()" here
				 * because it will remove the message.
				 * We'll do half of the cancel, then
				 * check if we need to run the new alert,
				 * and remove the message if not.
				 */
				pps->alert->active = 0;
				terminate (pps->alert->exec);
				if (NALERT.shcmd)
					alert (A_PRINTER, pps, (RSTATUS *)0, (char *)0);
				else
					Unlink (pps->alert->msgfile);
			}
#undef	OALERT
#undef	NALERT

		freeprinter (&op);

		unload_list (&pps->users_allowed);
		unload_list (&pps->users_denied);
		unload_list (&pps->forms_allowed);
		unload_list (&pps->forms_denied);
		load_userprinter_access (
			pp->name,
			&pps->users_allowed,
			&pps->users_denied
		);
		load_formprinter_access (
			pp->name,
			&pps->forms_allowed,
			&pps->forms_denied
		);

		load_sdn (&pps->cpi, pp->cpi);
		load_sdn (&pps->lpi, pp->lpi);
		load_sdn (&pps->plen, pp->plen);
		load_sdn (&pps->pwid, pp->pwid);

		pps->last_dial_rc = 0;
		pps->nretry = 0;

		/*
		 * Evaluate all requests queued for this printer,
		 * to make sure they are still eligible. They will
		 * get moved to another printer, get (re)filtered,
		 * or get canceled.
		 */
		(void)queue_repel (pps, 0, (int (*)())0);

		status = MOK;

	/*
	 * Room for new printer?
	 */
	} else if ((pps = search_ptable((char *)0))) {
		pps->status = PS_DISABLED | PS_REJECTED;
		pps->request = 0;
		pps->alert->active = 0;

		pps->form = 0;
		pps->pwheel_name = 0;
		pps->pwheel = 0;

		load_str (&pps->dis_reason, CUZ_NEW_PRINTER);
		load_str (&pps->rej_reason, CUZ_NEW_DEST);
		time (&pps->dis_date);
		time (&pps->rej_date);

		*(pps->printer) = *pp;

		untidbit (pp->printer_type);

		unload_list (&pps->users_allowed);
		unload_list (&pps->users_denied);
		unload_list (&pps->forms_allowed);
		unload_list (&pps->forms_denied);
		load_userprinter_access (
			pp->name,
			&pps->users_allowed,
			&pps->users_denied
		);
		load_formprinter_access (
			pp->name,
			&pps->forms_allowed,
			&pps->forms_denied
		);

		load_sdn (&pps->cpi, pp->cpi);
		load_sdn (&pps->lpi, pp->lpi);
		load_sdn (&pps->plen, pp->plen);
		load_sdn (&pps->pwid, pp->pwid);

		pps->last_dial_rc = 0;
		pps->nretry = 0;

		dump_pstatus ();

		status = MOK;

	} else {
		freeprinter (pp);
		status = MNOSPACE;
	}


	send (n, R_LOAD_PRINTER, status);
	return;
}

/**
 ** unload_printer() - S_UNLOAD_PRINTER
 **/

static void
_unload_printer (pps)
	PSTATUS			*pps;
{
	register CSTATUS	*pcs;


	if (pps->alert->active)
		cancel_alert (A_PRINTER, pps);

	/*
	 * Remove this printer from the classes it may be in.
	 * This is likely to be redundant, i.e. upon deleting
	 * a printer the caller is SUPPOSED TO check all the
	 * classes; any that contain the printer will be changed
	 * and we should receive a S_LOAD_CLASS message for each
	 * to reload the class.
	 *
	 * HOWEVER, this leaves a (small) window where someone
	 * can sneak a request in destined for the CLASS. If
	 * we have deleted the printer but still have it in the
	 * class, we may have trouble!
	 */
	for (pcs = walk_ctable(1); pcs; pcs = walk_ctable(0))
		(void)dellist (&(pcs->class->members), pps->printer->name);

	untidbit (pps->printer->printer_type);
	freeprinter (pps->printer);
	pps->printer->name = 0;		/* freeprinter() doesn't */

	return;
}

void
unload_printer (m, n)
	char			*m;
	NODE			*n;
{
	char			*printer;

	ushort			status;

	register PSTATUS	*pps;


	(void)getmessage (m, S_UNLOAD_PRINTER, &printer);

	/*
	 * Unload ALL printers?
	 */
	if (!*printer || STREQU(printer, NAME_ALL))

		/*
		 * If we have ANY requests queued, we can't do it.
		 */
		if (!Request_List)
			status = MBUSY;

		else {
			for (pps = walk_ptable(1); pps; pps = walk_ptable(0))
				_unload_printer (pps);
			status = MOK;
		}

	/*
	 * Have we seen this printer before?
	 */
	else if (!(pps = search_ptable(printer)))
		status = MNODEST;

	/*
	 * Can we can move all requests queued for this printer
	 * to another printer?
	 */
	else {
		/*
		 * Note: This routine WILL MOVE requests to another
		 * printer. It will not stop until it has gone through
		 * the entire list of requests, so all requests that
		 * can be moved will be moved. If any couldn't move,
		 * however, we don't unload the printer.
		 */
		if (queue_repel(pps, 1, (int (*)())0))
			status = MOK;
		else
			status = MBUSY;

		if (status == MOK)
			_unload_printer (pps);

	}

	if (status == MOK)
		dump_pstatus ();

	send (n, R_UNLOAD_PRINTER, status);
	return;
}

/**
 ** inquire_printer_status() - S_INQUIRE_PRINTER
 **/

void
inquire_printer_status (m, n)
	char			*m;
	NODE			*n;
{
	char			*printer;

	register PSTATUS	*pps,
				*ppsnext;


	(void)getmessage (m, S_INQUIRE_PRINTER_STATUS, &printer);

	/*
	 * Inquire about ALL printers?
	 */
	if (!*printer || STREQU(printer, NAME_ALL)) {
		if ((pps = walk_ptable(1)))
			for (; (ppsnext = walk_ptable(0)); pps = ppsnext)
				send (
					n,
					R_INQUIRE_PRINTER_STATUS,
					MOKMORE,
					pps->printer->name,
					(pps->form? pps->form->form->name : ""),
					(pps->pwheel_name? pps->pwheel_name : ""),
					pps->dis_reason,
					pps->rej_reason,
					pps->status,
					(pps->request? pps->request->secure->req_id : ""),
					pps->dis_date,
					pps->rej_date
				);

	/*
	 * Inquire about a specific printer?
	 */
	} else
		pps = search_ptable(printer);

	if (pps)
		send (
			n,
			R_INQUIRE_PRINTER_STATUS,
			MOK,
			pps->printer->name,
			(pps->form? pps->form->form->name : ""),
			(pps->pwheel_name? pps->pwheel_name : ""),
			pps->dis_reason,
			pps->rej_reason,
			pps->status,
			(pps->request? pps->request->secure->req_id : ""),
			pps->dis_date,
			pps->rej_date
		);

	else
		send (n,
			R_INQUIRE_PRINTER_STATUS,
			MNODEST,
			"",
			"",
			"",
			"",
			"",
			0,
			"",
			0L,
			0L
		);

	return;
}

/**
 ** load_class() - S_LOAD_CLASS
 **/

void
load_class (m, n)
	char			*m;
	NODE			*n;
{
	char			*class;

	ushort			status;

	register CLASS		*pc;

	register CSTATUS	*pcs;


	(void)getmessage (m, S_LOAD_CLASS, &class);

	if (!*class)
		status = MNODEST;

	/*
	 * Strange or missing class?
	 */
	else if (!(pc = Getclass(class))) {
		switch (errno) {
		case EBADF:
			status = MERRDEST;
			break;
		case ENOENT:
		default:
			status = MNODEST;
			break;
		}

	/*
	 * Class we already know about?
	 */
	} else if ((pcs = search_ctable(class))) {
		register RSTATUS	*prs;

		freeclass (pcs->class);
		*(pcs->class) = *pc;

		/*
		 * Here we go through the list of requests
		 * to see who gets affected.
		 */
		BEGIN_WALK_BY_DEST_LOOP (prs, class)
			/*
			 * If not still eligible for this class...
			 */
			switch (validate_request(prs, (char **)0, 1)) {
			case MOK:
			case MERRDEST:	/* rejecting (shouldn't happen) */
				break;
			case MDENYDEST:
			case MNOMOUNT:
			case MNOMEDIA:
			case MNOFILTER:
			default:
				/*
				 * ...then too bad!
				 */
				cancel (prs, 1);
				break;
			}
		END_WALK_LOOP
		status = MOK;

	/*
	 * Room for new class?
	 */
	} else if ((pcs = search_ctable((char *)0))) {
		pcs->status = CS_REJECTED;

		load_str (&pcs->rej_reason, CUZ_NEW_DEST);
		time (&pcs->rej_date);

		*(pcs->class) = *pc;

		dump_cstatus ();

		status = MOK;

	} else {
		freeclass (pc);
		status = MNOSPACE;
	}


	send (n, R_LOAD_CLASS, status);
	return;
}

/**
 ** unload_class() - S_UNLOAD_CLASS
 **/

static
void
_unload_class (pcs)
	CSTATUS			*pcs;
{
	freeclass (pcs->class);
	pcs->class->name = 0;	/* freeclass() doesn't */

	return;
}

void
unload_class (m, n)
	char			*m;
	NODE			*n;
{
	char			*class;

	ushort			status;

	RSTATUS			*prs;

	register CSTATUS	*pcs;


	(void)getmessage (m, S_UNLOAD_CLASS, &class);

	/*
	 * Unload ALL classes?
	 */
	if (!*class || STREQU(class, NAME_ALL)) {

		/*
		 * If we have a request queued for a member of ANY
		 * class, we can't do it.
		 */
		status = MOK;
		for (
			pcs = walk_ctable(1);
			pcs && status == MOK;
			pcs = walk_ctable(0)
		)
			BEGIN_WALK_BY_DEST_LOOP (prs, pcs->class->name)
				status = MBUSY;
				break;
			END_WALK_LOOP

		if (status == MOK)
			for (pcs = walk_ctable(1); pcs; pcs = walk_ctable(0))
				_unload_class (pcs);

	/*
	 * Have we seen this class before?
	 */
	} else if (!(pcs = search_ctable(class)))
		status = MNODEST;

	/*
	 * Is there even one request queued for this class?
	 * If not, we can safely remove it.
	 */
	else {
		status = MOK;
		BEGIN_WALK_BY_DEST_LOOP (prs, class)
			status = MBUSY;
			break;
		END_WALK_LOOP
		if (status == MOK)
			_unload_class (pcs);
	}

	if (status == MOK)
		dump_cstatus ();

	send (n, R_UNLOAD_CLASS, status);
	return;
}

/**
 ** inquire_class() - S_INQUIRE_CLASS
 **/

void
inquire_class (m, n)
	char			*m;
	NODE			*n;
{
	char			*class;

	register CSTATUS	*pcs,
				*pcsnext;


	(void)getmessage (m, S_INQUIRE_CLASS, &class);

	/*
	 * Inquire about ALL classes?
	 */
	if (!*class || STREQU(class, NAME_ALL)) {
		if ((pcs = walk_ctable(1)))
			for (; (pcsnext = walk_ctable(0)); pcs = pcsnext)
				send (
					n,
					R_INQUIRE_CLASS,
					MOKMORE,
					pcs->class->name,
					pcs->status,
					pcs->rej_reason,
					pcs->rej_date
				);

	/*
	 * Inquire about a single class?
	 */
	} else
		pcs = search_ctable(class);

	if (pcs)
		send (
			n,
			R_INQUIRE_CLASS,
			MOK,
			pcs->class->name,
			pcs->status,
			pcs->rej_reason,
			pcs->rej_date
		);

	else
		send (n, R_INQUIRE_CLASS, MNODEST, "", 0, "", 0L);

	return;
}
