/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/filters/putfilter.c	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "string.h"

#include "lp.h"
#include "filters.h"

extern char		*malloc(),
			*realloc();

extern void		free();

/**
 ** putfilter() - PUT FILTER INTO FILTER TABLE
 **/

int			putfilter (name, flbufp)
	char			*name;
	FILTER			*flbufp;
{
	_FILTER			_flbuf;

	register _FILTER	*pf;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		errno = EINVAL;
		return (-1);
	}

	_flbuf.name = strdup(name);
	_flbuf.command = (flbufp->command? strdup(flbufp->command) : 0);
	_flbuf.type = flbufp->type;
	_flbuf.printer_types = sl_to_typel(flbufp->printer_types);
	_flbuf.printers = duplist(flbufp->printers);
	_flbuf.input_types = sl_to_typel(flbufp->input_types);
	_flbuf.output_types = sl_to_typel(flbufp->output_types);
	if (!flbufp->templates)
		_flbuf.templates = 0;
	else if (!(_flbuf.templates = sl_to_templatel(flbufp->templates))) {
		freefilter (&_flbuf);
		errno = EBADF;
		return (-1);
	}

	if (!filters && get_and_load() == -1 && errno != ENOENT) {
		freefilter (&_flbuf);
		return (-1);
	}

	if (filters) {

		if ((pf = search_filter(name)))
			freefilter (pf);
		else {
			nfilters++;
			filters = (_FILTER *)realloc(
				(char *)filters,
				(nfilters + 1) * sizeof(_FILTER)
			);
			if (!filters) {
				freefilter (&_flbuf);
				errno = ENOMEM;
				return (-1);
			}
			filters[nfilters].name = 0;
			pf = filters + nfilters - 1;
		}

	} else {

		nfilters = 1;
		pf = filters = (_FILTER *)malloc(
			(nfilters + 1) * sizeof(_FILTER)
		);
		if (!filters) {
			freefilter (&_flbuf);
			errno = ENOMEM;
			return (-1);
		}
		filters[nfilters].name = 0;

	}

	*pf = _flbuf;

	return (dumpfilters((char *)0));
}
