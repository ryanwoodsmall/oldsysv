/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/okprinter.c	1.7"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "sys/types.h"

#include "lp.h"
#include "printers.h"

extern void		free();

static int		okinterface();

unsigned long		badprinter	= 0;

/**
 ** okprinter() - SEE IF PRINTER STRUCTURE IS SOUND
 **/

int			okprinter (name, prbufp, isput)
	register char		*name;
	register PRINTER	*prbufp;
	register int		isput;
{
	badprinter = 0;

#if	defined(REMOTE)
	/*
	 * A printer can't be remote and have device, interface,
	 * fault recovery, or alerts.
	 */
	if (
		prbufp->remote
	     && (
			prbufp->device
		     || prbufp->interface
		     || prbufp->fault_rec
		     || prbufp->fault_alert.shcmd
		)
	)
		badprinter |= BAD_REMOTE;
#endif

	/*
	 * A local printer must have an interface program. This is
	 * for historical purposes (it let's someone know where the
	 * interface program came from) AND is used by "putprinter()"
	 * to copy the interface program. We must be able to read it.
	 */
#if	defined(REMOTE)
	if (
		!prbufp->remote
	     && (
#else
	if ((
#endif
		isput
	     && !okinterface(name, prbufp)
	))
		badprinter |= BAD_INTERFACE;

	/*
	 * A local printer must have device or dial info.
	 */
#if	defined(REMOTE)
	if (
		!prbufp->remote
	     && (
#else
	if ((
#endif
	 	!prbufp->device
	     && !prbufp->dial_info
	))
		badprinter |= BAD_DEVDIAL;

	/*
	 * Fault recovery must be one of three kinds
	 * (or default).
	 */
	if (
		prbufp->fault_rec
	     && !STREQU(prbufp->fault_rec, NAME_CONTINUE)
	     && !STREQU(prbufp->fault_rec, NAME_BEGINNING)
	     && !STREQU(prbufp->fault_rec, NAME_WAIT)
	)
		badprinter |= BAD_FAULT;

	/*
	 * Alert command can't be reserved word.
	 */
	if (
		STREQU(prbufp->fault_alert.shcmd, NAME_QUIET)
	     || STREQU(prbufp->fault_alert.shcmd, NAME_LIST)
	)
		badprinter |= BAD_ALERT;

	return ((badprinter & ~ignprinter)? 0 : 1);
}

/**
 ** okinterface() - CHECK THAT THE INTERFACE PROGRAM IS OKAY
 **/

static int		canread (path)
	char			*path;
{
	FILE			*fp;

	if (!(fp = fopen(path, "r")))
		return (0);
	fclose (fp);
	return (1);
}

static int		okinterface (name, prbufp)
	char			*name;
	register PRINTER	*prbufp;
{
	int			ret;

	register char		*path;


	if (prbufp->interface)
		ret = canread(prbufp->interface);

	else
		if (!(path = makepath(Lp_A_Interfaces, name, (char *)0)))
			ret = 0;
		else {
			ret = canread(path);
			free (path);
		}

	return (ret);
}
