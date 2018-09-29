/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/freefilter.c	1.4"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lp.h"
#include "filters.h"

extern void		free();

/**
 ** freefilter() - FREE INTERNAL SPACE ALLOCATED FOR A FILTER
 **/

static void		freetypel (typel)
	register TYPE		*typel;
{
	register TYPE		*pt;

	if (typel) {
		for (pt = typel; pt->name; pt++)
			free (pt->name);
		free ((char *)typel);
	}
	return;
}

void			freetempl (templ)
	register TEMPLATE	*templ;
{
	register TEMPLATE	*pt;

	if (templ) {
		for (pt = templ; pt->keyword; pt++) {
			free (pt->keyword);
			if (pt->pattern)
				free (pt->pattern);
#if	defined(PATT_RE)
			if (pt->re)
				free (pt->re);
#endif
			if (pt->result)
				free (pt->result);
		}
		free ((char *)templ);
	}
	return;
}

void			freefilter (pf)
	_FILTER			*pf;
{
	if (!pf)
		return;
	if (pf->name)
		free (pf->name);
	if (pf->command)
		free (pf->command);
	freelist (pf->printers);
	freetypel (pf->printer_types);
	freetypel (pf->input_types);
	freetypel (pf->output_types);
	freetempl (pf->templates);

	return;
}
