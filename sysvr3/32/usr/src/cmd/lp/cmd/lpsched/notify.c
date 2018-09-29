/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/notify.c	1.2"

#include "lpsched.h"

#include "varargs.h"

static char		*N_Msg[] = {
	"Subject: Status of lp request %s\n\nYour request %s destined for %s%s\n",
	"has completed successfully on printer %s.\n",
	"was canceled by the administrator%s\n",
	"encountered an error during filtering.\n",
	"encountered an error while printing on printer %s.\n",
	"Filtering stopped with an exit code of %d.\n",
	"Printing stopped with an exit code of %d.\n",
	"Filtering was interrupted with a signal %d.\n",
	"Printing was interrupted with a signal %d.\n",
	"\nReason for failure:\n\n%s\n",
	"\nReason for being canceled:\n\n%s\n",
};

static struct reason {
	short			reason;
	char			*msg;
}			N_Reason[] = {
    {
	MNODEST,
	"The requested print destination has been removed."
    }, {
	MERRDEST,
	"All candidate destinations are rejecting further requests."
    }, {
	MDENYDEST,
	"You are no longer allowed to use any printer suitable for\nthe request."
    }, {
	MDENYDEST,
	"No candidate printer can handle these characteristics:"
    }, {
	MNOMEDIA,
	"The form you requested no longer exists."
    }, {
	MDENYMEDIA,
	"You are no longer allowed to use the form you requested."
    }, {
	MDENYMEDIA,
	"The form you wanted now requires a different character set."
    }, {
	MNOFILTER,
	"There is no longer a filter that will convert your file for printing."
    }, {
	MNOMOUNT,
	"The form or print wheel you requested is not allowed on any\nprinter otherwise suitable for the request."
    }, {
	MNOSPACE,
	"Memory allocation problem."
    }, {
	-1,
	""
    }
};
	
static void             print_reason();

/**
 ** notify() - NOTIFY USER OF FINISHED REQUEST
 **/
	
void                    notify (prs, errbuf, k, e, slow)
	register RSTATUS        *prs;
	char                    *errbuf;
	int                     k,
				e,
				slow;
{
	register char		*cp;

	char			*file;

	FILE                    *fp;


	/*
	 * Screen out cases where no notification is needed.
	 */
	if (!(prs->request->outcome & RS_NOTIFY))
		return;
	if (
		!(prs->request->actions & (ACT_MAIL|ACT_WRITE))
	     && !prs->request->alert
	     && !(prs->request->outcome & RS_CANCELLED)
	     && !e && !k && !file       /* exited normally */
	)
		return;

	/*
	 * Create the notification message to the user.
	 */
	file = makereqerr(prs->secure->req_id);
	if ((fp = open_lpfile(file, "w", MODE_NOREAD))) {
		fprintf (
			fp,
			N_Msg[0],
			prs->secure->req_id,
			prs->secure->req_id,
			prs->request->destination,
			STREQU(prs->request->destination, NAME_ANY)?
				  " printer"
				: ""
		);
	
		if (prs->request->outcome & RS_PRINTED)
			fprintf (
				fp,
				N_Msg[1],
				prs->printer->printer->name
			);

		if (prs->request->outcome & RS_CANCELLED)
			fprintf (
				fp,
				N_Msg[2],
				(prs->request->outcome & RS_FAILED)?
					  ", and"
					: "."
			);
	
		if (prs->request->outcome & RS_FAILED) {
			if (slow)
				fputs (N_Msg[3], fp);
			else
				fprintf (
					fp,
					N_Msg[4],
					prs->printer->printer->name
				);
	
			if (e > 0)
				fprintf (fp, N_Msg[slow? 5 : 6], e);
			else if (k)
				fprintf (fp, N_Msg[slow? 7 : 8], k);
		}
	
		if (errbuf) {
			for (cp = errbuf; *cp && *cp == '\n'; cp++)
				;
			fprintf (fp, N_Msg[9], cp);
			if (prs->request->outcome & RS_CANCELLED)
				fputs ("\n", fp);
		}

		if (prs->request->outcome & RS_CANCELLED)
			print_reason (fp, prs->reason);

		close_lpfile (fp);
		schedule (EV_NOTIFY, prs);

	}
	if (file)
		free (file);

	return;
}

/**
 ** print_reason() - PRINT REASON FOR AUTOMATIC CANCEL
 **/

static void		print_reason (fp, reason)
	FILE			*fp;
	register int		reason;
{
	register int		i;


#define P(BIT,MSG)	if (chkprinter_result & BIT) fputs (MSG, fp)

	for (i = 0; N_Reason[i].reason != -1; i++)
		if (N_Reason[i].reason == reason) {
			if (reason == MDENYDEST && chkprinter_result)
				i++;
			if (reason == MDENYMEDIA && chkprinter_result)
				i++;
			fprintf (fp, N_Msg[10], N_Reason[i].msg);
			if (reason == MDENYDEST && chkprinter_result) {
				P (PCK_TYPE,	"\tprinter type\n");
				P (PCK_CHARSET,	"\tcharacter set\n");
				P (PCK_CPI,	"\tcharacter pitch\n");
				P (PCK_LPI,	"\tline pitch\n");
				P (PCK_WIDTH,	"\tpage width\n");
				P (PCK_LENGTH,	"\tpage length\n");
				P (PCK_BANNER,	"\tno banner\n");
			}
			break;
		}

	return;
}
