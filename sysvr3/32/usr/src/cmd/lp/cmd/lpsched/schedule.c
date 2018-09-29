/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/schedule.c	1.15"

#include "varargs.h"

#include "lpsched.h"

typedef struct later {
	struct later		*next;
	int			event,
				ticks;
	union arg {
		PSTATUS			*printer;
		RSTATUS			*request;
	}			arg;
}			LATER;

static LATER		LaterHead	= { 0 },
			TempHead;

static void		ev_interf();
extern void		ev_message();

static int		ev_slowf(),
			ev_notify();

static EXEC		*find_exec_slot();

/**
 ** schedule() - SCHEDULE BY EVENT
 **/

/*VARARGS1*/
void			schedule (event, va_alist)
	int			event;
	va_dcl
{
	va_list			ap;

	LATER			*plprev,
				*pl,
				*plnext;

	int			do_ev_message;
#if	defined(CHECK_CHILDREN)
	int			do_ev_checkchild;
#endif

	register PSTATUS	*pps;

	register RSTATUS	*prs;


	TRACE_ON ("schedule");

	va_start (ap);

	/*
	 * If we're still in the process of starting up, don't start
	 * anything! Schedule it for one tick later. While we're starting
	 * ticks aren't counted, so the events won't be started.
	 * HOWEVER, with a count of 1, a single EV_ALARM after we're
	 * finished starting will be enough to clear all things scheduled
	 * for later.
	 */
	if (Starting) {
		switch (event) {

		case EV_INTERF:
			pps = va_arg(ap, PSTATUS *);
			schedule (EV_LATER, 1, EV_INTERF, pps);
			goto Return;

		case EV_SLOWF:
		case EV_NOTIFY:
			prs = va_arg(ap, RSTATUS *);
			schedule (EV_LATER, 1, event, prs);
			goto Return;

		case EV_LATER:
			/*
			 * This is okay--in fact it may be us!
			 */
			break;

		case EV_ALARM:
			/*
			 * The alarm will go off again, hold off for now.
			 */
			goto Return;

		}
	}

	/*
	 * Schedule something:
	 */
	switch (event) {

	case EV_INTERF:
		if ((pps = va_arg(ap, PSTATUS *)))
			ev_interf (pps);

		else
			for (
				pps = walk_ptable(1);
				pps;
				pps = walk_ptable(0)
			)
				ev_interf (pps);

		break;

	case EV_SLOWF:
		if ((prs = va_arg(ap, RSTATUS *)))
			(void)ev_slowf (prs);
		else
			for (
				prs = Request_List;
				prs && ev_slowf(prs) != -1;
				prs = prs->next
			)
				;
		break;

	case EV_NOTIFY:
		if ((prs = va_arg(ap, RSTATUS *)))
			(void)ev_notify (prs);
		else
			for (
				prs = Request_List;
				prs && ev_notify(prs) != -1;
				prs = prs->next
			)
				;
		break;

	case EV_LATER:
		if ((pl = (LATER *)malloc(sizeof(LATER)))) {
			if (!LaterHead.next)
				alarm (CLOCK_TICK);

			pl->next = LaterHead.next;
			LaterHead.next = pl;

			pl->ticks = va_arg(ap, int);
			pl->event = va_arg(ap, int);
			switch (pl->event) {

			case EV_INTERF:
				pl->arg.printer = va_arg(ap, PSTATUS *);
				if (pl->arg.printer)
					pl->arg.printer->status |= PS_LATER;
				break;

			case EV_SLOWF:
			case EV_NOTIFY:
				pl->arg.request = va_arg(ap, RSTATUS *);

#if	defined(CHECK_CHILDREN)
			case EV_CHECKCHILD:
#endif	/* CHECK_CHILDREN */
			case EV_MESSAGE:
				break;

			}
		}
		break;

	case EV_ALARM:
		Sig_Alrm = 0;

		/*
		 * The act of scheduling some of the ``laters'' may
		 * cause new ``laters'' to be added to the list.
		 * To ease the handling of the linked list, we first
		 * run through the list and move all events ready to
		 * be scheduled to another list. Then we schedule the
		 * events off the new list. This leaves the main ``later''
		 * list ready for new events.
		 */
		TempHead.next = 0;
		for (
			pl = (plprev = &LaterHead)->next;
			pl;
			pl = plnext
		) {
			plnext = pl->next;
			if (--pl->ticks)
				plprev = pl;
			else {
				plprev->next = plnext;

				pl->next = TempHead.next;
				TempHead.next = pl;
			}
		}

		do_ev_message = 0;
		for (pl = TempHead.next; pl; pl = plnext) {
			plnext = pl->next;
			switch (pl->event) {

			case EV_INTERF:
				pl->arg.printer->status &= ~PS_LATER;
				schedule (pl->event, pl->arg.printer);
				break;

			case EV_SLOWF:
			case EV_NOTIFY:
				schedule (pl->event, pl->arg.request);
				break;

			case EV_MESSAGE:
				/*
				 * Do this one only once.
				 */
				do_ev_message = 1;
				break;

#if	defined(CHECK_CHILDREN)
			case EV_CHECKCHILD:
				/*
				 * Do this one only once.
				 */
				do_ev_checkchild = 1;
				break;
#endif	/* CHECK_CHILDREN */

			}
			free ((char *)pl);
		}
		if (do_ev_message)
			schedule (EV_MESSAGE);

#if	defined(CHECK_CHILDREN)
		if (do_ev_checkchild)
			schedule (EV_CHECKCHILD);
#endif	/* CHECK_CHILDREN */

		if (LaterHead.next)
			alarm (CLOCK_TICK);
		break;

	case EV_MESSAGE:
		ev_message ();
		break;

#if	defined(CHECK_CHILDREN)
	case EV_CHECKCHILD:
		ev_checkchild ();
		break;
#endif	/* CHECK_CHILDREN */

	}

Return:	va_end (ap);

	TRACE_OFF ("schedule");
	return;
}

/**
 ** maybe_schedule() - MAYBE SCHEDULE SOMETHING FOR A REQUEST
 **/

void			maybe_schedule (prs)
	RSTATUS			*prs;
{
	/*
	 * Use this routine if a request has been changed by some
	 * means so that it is ready for filtering or printing,
	 * but a previous filtering or printing process for this
	 * request MAY NOT have finished yet. If a process is still
	 * running, then the cleanup of that process will cause
	 * "schedule()" to be called. Calling "schedule()" regardless
	 * might make another request slip ahead of this request.
	 */

	/*
	 * "schedule()" will refuse if this request is filtering.
	 * It will also refuse if the request ``was'' filtering
	 * but the filter was terminated in "validate_request()",
	 * because we can not have heard from the filter process
	 * yet. Also, when called with a particular request,
	 * "schedule()" won't slip another request ahead.
	 */
	if (NEEDS_FILTERING(prs))
		schedule (EV_SLOWF, prs);

	else if (!(prs->request->outcome & RS_STOPPED))
		schedule (EV_INTERF, prs->printer);

	return;
}

/**
 ** ev_interf() - CHECK AND EXEC INTERFACE PROGRAM 
 **/

/*
 * Macro to check if the request needs a print wheel or character set (S)
 * and the printer (P) has it mounted or can select it. Since the request
 * has already been approved for the printer, we don't have to check the
 * character set, just the mount. If the printer has selectable character
 * sets, there's nothing to check so the request is ready to print.
 */
#define MATCH(PRS,PPS) \
	( \
		!(PPS)->printer->daisy \
	     || !(PRS)->pwheel_name \
	     || !((PRS)->request->outcome & RS_PWMAND) \
	     || STREQU((PRS)->pwheel_name, NAME_ANY) \
	     || ( \
			(PPS)->pwheel_name \
		     && STREQU((PPS)->pwheel_name, (PRS)->pwheel_name) \
		) \
	)

static void		ev_interf (pps)
	PSTATUS			*pps;
{
	register RSTATUS	*prs;


	/*
	 * If the printer isn't tied up doing something
	 * else, and isn't disabled, see if there is a request
	 * waiting to print on it. Note: We don't include
	 * PS_FAULTED here, because simply having a printer
	 * fault (without also being disabled) isn't sufficient
	 * to keep us from trying again. (In fact, we HAVE TO
	 * try again, to see if the fault has gone away.)
	 *
	 * NOTE: If the printer is faulted but the filter controlling
	 * the printer is waiting for the fault to clear, a
	 * request will still be attached to the printer, as
	 * evidenced by "pps->request", so we won't try to
	 * schedule another request!
	 */
	if (pps->request || pps->status & (PS_DISABLED|PS_LATER|PS_BUSY))
		return;

	BEGIN_WALK_BY_PRINTER_LOOP (prs, pps)
		/*
		 * Just because the printer isn't busy and the
		 * request is assigned to this printer, don't get the
		 * idea that the request can't be printing (RS_ACTIVE),
		 * because another printer may still have the request
		 * attached but we've not yet heard from the child
		 * process controlling that printer.
		 */
		if (
			!(prs->request->outcome & (RS_DONE|RS_HELD|RS_ACTIVE))
		     && !NEEDS_FILTERING(prs)
		     && prs->form == pps->form
		     && MATCH(prs, pps)
		) {
			/*
			 * We have the waiting request, we have
			 * the ready printer. If the exec fails
			 * because the fork failed, schedule a
			 * try later and claim we succeeded. The
			 * later attempt will sort things out,
			 * e.g. will re-schedule if the fork fails
			 * again.
			 */
			pps->request = prs;
			if (exec(EX_INTERF, pps) == 0) {
				pps->status |= PS_BUSY;
				return;
			}
			pps->request = 0;
			if (errno == EAGAIN) {
				load_str (&pps->dis_reason, CUZ_NOFORK);
				schedule (EV_LATER, WHEN_FORK, EV_INTERF, pps);
				return;
			}
		}
	END_WALK_LOOP

	return;
}

/**
 ** ev_slowf() - CHECK AND EXEC SLOW FILTER
 **/

static int		ev_slowf (prs)
	register RSTATUS	*prs;
{
	register EXEC		*ep;


	/*
	 * Return -1 if no more can be executed (no more exec slots)
	 * or if it's unwise to execute any more (fork failed).
	 */

	if (!(ep = find_exec_slot(Exec_Slow, ET_SlowSize)))
		return (-1);

	if (
		!(prs->request->outcome & (RS_DONE|RS_HELD|RS_ACTIVE))
	     && NEEDS_FILTERING(prs)
	) {
	 	(prs->exec = ep)->ex.request = prs;
		if (exec(EX_SLOWF, prs) != 0) {
			ep->ex.request = 0;
			prs->exec = 0;
			if (errno == EAGAIN) {
				schedule (EV_LATER, WHEN_FORK, EV_SLOWF, prs);
				return (-1);
			}
		}
	}
	return (0);
}

/**
 ** ev_notify() - CHECK AND EXEC NOTIFICATION
 **/

static int		ev_notify (prs)
	register RSTATUS	*prs;
{
	register EXEC		*ep;

	/*
	 * Return -1 if no more can be executed (no more exec slots)
	 * or if it's unwise to execute any more (fork failed).
	 */

	if (!(ep = find_exec_slot(Exec_Notify, ET_NotifySize)))
		return (-1);

	if (
		prs->request->outcome & RS_NOTIFY
	     && !(prs->request->outcome & RS_NOTIFYING)
	) {
	 	(prs->exec = ep)->ex.request = prs;
		if (exec(EX_NOTIFY, prs) != 0) {
			ep->ex.request = 0;
		 	prs->exec = 0;
			if (errno == EAGAIN) {
				schedule (EV_LATER, WHEN_FORK, EV_NOTIFY, prs);
				return (-1);
			}
		}
	}
	return (0);
}

/**
 ** ev_message() - RETRY A PREVIOUSLY FAILED MESSAGE
 **/

/*
 * This routine is defined in the messages handling source.
 */

/**
 ** find_exec_slot() - FIND AVAILABLE EXEC SLOT
 **/

static EXEC		*find_exec_slot (exec_table, size)
	register EXEC		*exec_table;
	int			size;
{
	register EXEC		*ep,
				*last_ep	= exec_table + size - 1;

	for (ep = exec_table; ep <= last_ep; ep++)
		if (ep->pid == 0)
			return (ep);

	return (0);
}
