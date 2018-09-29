/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/printwheels.c	1.6"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/stat.h"

#include "lp.h"
#include "fs.h"
#include "printers.h"

extern void		free();

/**
 ** getpwheel() - GET PRINT WHEEL INFO FROM DISK
 **/

PWHEEL			*getpwheel (name)
	char			*name;
{
	static long		lastdir		= -1;

	static PWHEEL		pwheel;

	register FALERT		*pa;


	if (!name || !*name) {
		errno = EINVAL;
		return (0);
	}

	if (!Lp_A_PrintWheels) {
		getadminpaths (LPUSER);
		if (!Lp_A_PrintWheels)
			return (0);
	}

	/*
	 * Getting ``all''? If so, jump into the directory
	 * wherever we left off.
	 */
	if (STREQU(NAME_ALL, name)) {
		if (!(name = next_dir(Lp_A_PrintWheels, &lastdir)))
			return (0);
	} else
		lastdir = -1;

	/*
	 * Get the information for the alert.
	 */
	if (!(pa = getalert(Lp_A_PrintWheels, name))) {

		/*
		 * Unless the world has turned weird, we shouldn't
		 * get ENOTDIR if we're doing the ``all'' case--because
		 * getting here in the all case meant the printwheel
		 * directory exists, but ENOTDIR means it doesn't!
		 */
		if (errno == ENOTDIR)
			errno = ENOENT; /* printwheel doesn't exist */

		return (0);
	}

	pwheel.alert = *pa;
	pwheel.name = strdup(name);

	return (&pwheel);
}

/**
 ** putpwheel() - PUT PRINT WHEEL INFO TO DISK
 **/

int			putpwheel (name, pwheelp)
	char			*name;
	PWHEEL			*pwheelp;
{
	register char		*path;

	struct stat		statbuf;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(name, NAME_ALL)) {
		errno = ENOENT;
		return (-1);
	}

	if (!Lp_A_PrintWheels) {
		getadminpaths (LPUSER);
		if (!Lp_A_PrintWheels)
			return (0);
	}

	/*
	 * Create the parent directory for this printer
	 * if it doesn't yet exist.
	 */
	if (!(path = makepath(Lp_A_PrintWheels, name, (char *)0)))
		return (-1);
	if (Stat(path, &statbuf) == 0) {
		if (!(statbuf.st_mode & S_IFDIR)) {
			free (path);
			errno = ENOTDIR;
			return (-1);
		}
	} else if (errno != ENOENT || mkdir_lpdir(path, MODE_DIR) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	/*
	 * Now write out the alert condition.
	 */
	if (putalert(Lp_A_PrintWheels, name, &(pwheelp->alert)) == -1)
		return (-1);

	return (0);
}

/**
 ** delpwheel() - DELETE PRINT WHEEL INFO FROM DISK
 **/

int			delpwheel (name)
	char			*name;
{
	long			lastdir;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (!Lp_A_PrintWheels) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers)
			return (0);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		while ((name = next_dir(Lp_A_PrintWheels, &lastdir)))
			if (_delpwheel(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delpwheel(name));
}

/**
 ** _delpwheel()
 **/

static int		_delpwheel (name)
	char			*name;
{
	register char		*path;

	delalert (Lp_A_PrintWheels, name);

	if (!(path = makepath(Lp_A_PrintWheels, name, (char *)0)))
		return (-1);
	if (Rmdir(path)) {
		free (path);
		return (-1);
	}
	free (path);
	return (0);
}

/**
 **  freepwheel() - FREE MEMORY ALLOCATED FOR PRINT WHEEL STRUCTURE
 **/

void			freepwheel (ppw)
	PWHEEL			*ppw;
{
	if (!ppw)
		return;
	if (ppw->name)
		free (ppw->name);
	if (ppw->alert.shcmd)
		free (ppw->alert.shcmd);
	return;
}
