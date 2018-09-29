/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/pickfilter.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lpsched.h"

#include "validate.h"

/**
 ** pickfilter() - SEE IF WE CAN FIND A FILTER FOR THIS REQUEST
 **/

int			pickfilter (prs, pc, pfs)
	register RSTATUS	*prs;
	register CANDIDATE	*pc;
	register FSTATUS	*pfs;
{
	register char		**pp,
				**pl;

	register PSTATUS	*pps		= pc->pps;

	char			**modes		= 0,
				**parms		= 0,
				*default_type;

	FILTERTYPE		ret		= fl_none;

	char			*pipes[2],
				*cp;

	int			got_cpi		= 0,
				got_lpi		= 0,
				got_plen	= 0,
				got_pwid	= 0,
				must_have_filter= 0;


	TRACE_ON ("pick_filter");

	/*
	 * The bulk of the code below is building a parameter list "parms"
	 * to send to "insfilter()".
	 */

	if (prs->request->modes) {
		if ((cp = strdup(prs->request->modes))) {
			modes = getlist(cp, LP_WS, LP_SEP);
			free (cp);
		}
	}

	pp = parms = (char **)malloc(
		2 * (NPARM_SPEC + lenlist(modes) + 1) * sizeof(char *)
	);
	if (!parms)
		goto Return;

	/*
	 * If a form is involved, pick up its page size and print
	 * spacing requirements.
	 */
	if (pfs) {
		if (pfs->cpi) {
			*pp++ = PARM_CPI;
			*pp++ = pfs->cpi;
			got_cpi = 1;
		}
		if (pfs->lpi) {
			*pp++ = PARM_LPI;
			*pp++ = pfs->lpi;
			got_lpi = 1;
		}
		if (pfs->plen) {
			*pp++ = PARM_LENGTH;
			*pp++ = pfs->plen;
			got_plen = 1;
		}
		if (pfs->pwid) {
			*pp++ = PARM_WIDTH;
			*pp++ = pfs->pwid;
			got_pwid = 1;
		}

	/*
	 * If no form is involved, pick up whatever page size and print
	 * spacing requirements were given by the user.
	 */
	} else {
		if (o_cpi) {
			*pp++ = PARM_CPI;
			*pp++ = o_cpi;
			got_cpi = 1;
		}
		if (o_lpi) {
			*pp++ = PARM_LPI;
			*pp++ = o_lpi;
			got_lpi = 1;
		}
		if (o_length) {
			*pp++ = PARM_LENGTH;
			*pp++ = o_length;
			got_plen = 1;
		}
		if (o_width) {
			*pp++ = PARM_WIDTH;
			*pp++ = o_width;
			got_pwid = 1;
		}
	}

	/*
	 * Pick up whatever page size and print spacing requirements
	 * haven't been specified yet from the printer defaults.
	 */
	if (!got_cpi && pps->cpi) {
		*pp++ = PARM_CPI;
		*pp++ = pps->cpi;
	}
	if (!got_lpi && pps->lpi) {
		*pp++ = PARM_LPI;
		*pp++ = pps->lpi;
	}
	if (!got_plen && pps->plen) {
		*pp++ = PARM_LENGTH;
		*pp++ = pps->plen;
	}
	if (!got_pwid && pps->pwid) {
		*pp++ = PARM_WIDTH;
		*pp++ = pps->pwid;
	}

	/*
	 * Pick up the number of pages, character set (the form's
	 * or the user's), the form name, the number of copies,
	 * and the modes.
	 */

	if (prs->request->pages) {
		*pp++ = PARM_PAGES;
		*pp++ = prs->request->pages;
		must_have_filter = 1;
	}

	if (prs->request->charset) {
		*pp++ = PARM_CHARSET;
		*pp++ = prs->request->charset;

	} else if (pfs && pfs->form->chset) {
		*pp++ = PARM_CHARSET;
		*pp++ = pfs->form->chset;
	}

	if (prs->request->form) {
		*pp++ = PARM_FORM;
		*pp++ = prs->request->form;
	}

#define BIGGEST	"4294967296"	/* the biggest 32-bit integer */
	if (prs->request->copies > 1) {
		*pp++ = PARM_COPIES;
		sprintf ((*pp++ = BIGGEST), "%d", prs->request->copies);
	}

	if (modes) {
		for (pl = modes; *pl; pl++) {
			*pp++ = PARM_MODES;
			*pp++ = *pl;
		}
		must_have_filter = 1;
	}

	*pp = 0;	/* null terminated list! */


	pc->fast = 0;
	pc->slow = 0;
	pc->flags = 0;
	ret = fl_none;

	/*
	 * If the printer type is defined and not "unknown", then
	 * it is the type of choice to match. Otherwise, if input
	 * types have been given just check those. If nothing has
	 * been said about types for this printer, assume "simple".
	 */
	if (
		pps->printer->printer_type
	     && !STREQU(pps->printer->printer_type, NAME_UNKNOWN)
	)
		default_type = pps->printer->printer_type;

	else if (pps->printer->input_types)
		default_type = 0;

	else
		default_type = NAME_SIMPLE;

	/*
	 * If we don't really need a filter and the types match,
	 * then that's good enough. Some ``broadly defined''
	 * filters might match our needs, but if the printer
	 * can do what we need, then why pull in a filter?
	 * Besides, Section 3.40 in the requirements imply
	 * that we don't use a filter if the printer can handle
	 * the file.
	 */
	if (!must_have_filter) {

		if (SAME(default_type, prs->request->input_type))
			ret = fl_both;

		else if (pps->printer->input_types)
		    for (pl = pps->printer->input_types; *pl; pl++)
			if (
				STREQU(prs->request->input_type, *pl)
			     || STREQU(NAME_ANY, *pl)
			     || STREQU(NAME_ALL, *pl)
			     || (
					STREQU(NAME_TERMINFO, *pl)
				     && isterminfo(prs->request->input_type)
				)
			) {
				ret = fl_both;	/* not really, but ok */
				break;
			}
	}

	/*
	 * Don't try using a filter if the user doesn't want
	 * a filter to be used! He or she would rather see an
	 * error message than (heaven forbid!) a filter being
	 * used.
	 */
	if (ret == fl_none && !(prs->request->actions & ACT_RAW)) {

		/*
		 * For the printer type, and each input type the printer
		 * accepts, see if we have a filter that matches the
		 * request to the printer.
		 */
		if (default_type)
			ret = insfilter(
				pipes,
				prs->request->input_type,
				default_type,
				pps->printer->printer_type,
				pps->printer->name,
				parms,
				&(pc->flags)
			);
		if (ret == fl_none && pps->printer->input_types)
		    for (pl = pps->printer->input_types; *pl; pl++) {
			if (!SAME(default_type, *pl))
				ret = insfilter(
					pipes,
					prs->request->input_type,
					*pl,
					pps->printer->printer_type,
					pps->printer->name,
					parms,
					&(pc->flags)
				);
			if (ret != fl_none)
				break;
		    }

		if (ret == fl_both || ret == fl_slow)
			pc->slow = pipes[0];
		if (ret == fl_both || ret == fl_fast)
			pc->fast = pipes[1];

	}

Return:	if (parms)
		free ((char *)parms);
	if (modes)
		freelist (modes);

	TRACE_OFF ("pick_filter");
	return ((ret != fl_none));
}
