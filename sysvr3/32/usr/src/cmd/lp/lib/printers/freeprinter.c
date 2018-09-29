/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/freeprinter.c	1.2"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"

#include "lp.h"
#include "printers.h"

extern void		free();

/**
 **  freeprinter() - FREE MEMORY ALLOCATED FOR PRINTER STRUCTURE
 **/

void			freeprinter (pp)
	PRINTER			*pp;
{
	if (!pp)
		return;
	if (pp->name)
		free (pp->name);
	if (pp->char_sets)
		freelist (pp->char_sets);
	if (pp->input_types)
		freelist (pp->input_types);
	if (pp->device)
		free (pp->device);
	if (pp->dial_info)
		free (pp->dial_info);
	if (pp->fault_rec)
		free (pp->fault_rec);
	if (pp->interface)
		free (pp->interface);
	if (pp->printer_type)
		free (pp->printer_type);
	if (pp->remote)
		free (pp->remote);
	if (pp->speed)
		free (pp->speed);
	if (pp->stty)
		free (pp->stty);
	if (pp->description)
		free (pp->description);
	if (pp->fault_alert.shcmd)
		free (pp->fault_alert.shcmd);
	return;
}
