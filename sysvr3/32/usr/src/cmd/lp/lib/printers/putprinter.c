/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/putprinter.c	1.10"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"
#include "sys/stat.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"

#include "lp.h"
#include "printers.h"

extern char		*malloc();

extern void		free();

extern struct {
	char			*v;
	short			len,
				okremote;
}			prtrheadings[];

static void		print_sdn(),
			print_l(),
			print_str();

unsigned long		ignprinter	= 0;

/**
 ** putprinter() - WRITE PRINTER STRUCTURE TO DISK FILES
 **/

int			putprinter (name, prbufp)
	char			*name;
	PRINTER			*prbufp;
{
	register char		*path,
				*stty,
				*speed;

	FILE			*fp,
				*fpin;

	int			fld;

	char			buf[BUFSIZ];

	struct stat		statbuf1,
				statbuf2;


	badprinter = 0;

	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * First go through the structure and see if we have
	 * anything strange.
	 */
	if (!okprinter(name, prbufp, 1)) {
		errno = EINVAL;
		return (-1);
	}

	if (!Lp_A_Printers || !Lp_A_Interfaces) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers || !Lp_A_Interfaces)
			return (0);
	}

	/*
	 * Create the parent directory for this printer
	 * if it doesn't yet exist.
	 */
	if (!(path = getprinterfile(name, (char *)0)))
		return (-1);
	if (Stat(path, &statbuf1) == 0) {
		if (!(statbuf1.st_mode & S_IFDIR)) {
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
	 * Create the copy of the interface program, unless
	 * that would be silly or not desired.
	 */
	if (prbufp->interface && (ignprinter & BAD_INTERFACE) == 0) {
		if (Stat(prbufp->interface, &statbuf1) == -1)
			return (-1);
		if (!(path = makepath(Lp_A_Interfaces, name, (char *)0)))
			return (-1);
		if (
			Stat(path, &statbuf2) == -1
		     || statbuf1.st_dev != statbuf2.st_dev
		     || statbuf1.st_ino != statbuf2.st_ino
		) {
			register int		n;

			if (!(fpin = open_lpfile(prbufp->interface, "r"))) {
				free (path);
				return (-1);
			}
			if (!(fp = open_lpfile(path, "w", MODE_EXEC))) {
				free (path);
				close_lpfile (fpin);
				return (-1);
			}
			while ((n = fread(buf, 1, BUFSIZ, fpin)) > 0)
				fwrite (buf, 1, n, fp);
			close_lpfile (fp);
			close_lpfile (fpin);
		}
		free (path);
	}

	/*
	 * If this printer is dialed up, remove any baud rates
	 * from the stty option list and move the last one to
	 * the ".speed" member if the ".speed" member isn't already
	 * set. Conversely, if this printer is directly connected,
	 * move any value from the ".speed" member to the stty list. 
	 */

	stty = (prbufp->stty? strdup(prbufp->stty) : 0);
	speed = prbufp->speed;

	if (prbufp->dial_info && stty) {
		register char		*newstty,
					*p,
					*q;

		register int		len;

		if (!(q = newstty = malloc(strlen(stty) + 1))) {
			free (stty);
			errno = ENOMEM;
			return (-1);
		}
		newstty[0] = 0;	/* start with empty copy */

		for (
			p = strtok(stty, " ");
			p;
			p = strtok((char *)0, " ")
		) {
			len = strlen(p);
			if (strspn(p, "0123456789") == len) {
				/*
				 * If "prbufp->speed" isn't set, then
				 * use the speed we just found. Don't
				 * check "speed", because if more than
				 * one speed was given in the list, we
				 * want the last one.
				 */
				if (!prbufp->speed)
					speed = p;

			} else {
				/*
				 * Not a speed, so copy it to the
				 * new stty string.
				 */
				if (q != newstty)
					*q++ = ' ';
				strcpy (q, p);
				q += len;
			}
		}

		free (stty);
		stty = newstty;

	} else if (!prbufp->dial_info && speed) {
		register char		*newstty;

		newstty = malloc(strlen(stty) + 1 + strlen(speed) + 1);
		if (!newstty) {
			if (stty)
				free (stty);
			errno = ENOMEM;
			return (-1);
		}

		if (stty) {
			strcpy (newstty, stty);
			strcat (newstty, " ");
			strcat (newstty, speed);
			free (stty);
		} else
			strcpy (newstty, speed);
		speed = 0;

		stty = newstty;

	}

	/*
	 * Open the configuration file and write out the printer
	 * configuration.
	 */

	if (!(path = getprinterfile(name, CONFIGFILE))) {
		if (stty)
			free (stty);
		return (-1);
	}
	if (!(fp = open_lpfile(path, "w", MODE_READ))) {
		free (path);
		if (stty)
			free (stty);
		return (-1);
	}
	free (path);

	for (fld = 0; fld < PR_MAX; fld++) {

		if (prbufp->remote && !prtrheadings[fld].okremote)
			continue;

		switch (fld) {

#define HEAD	prtrheadings[fld].v

		case PR_BAN:
			FPRINTF (
				fp,
				"%s %s",
				HEAD,
				WHEN prbufp->banner & BAN_OFF
				USE NAME_OFF
				OTHERWISE NAME_ON
				NEHW
			);
			if (prbufp->banner & BAN_ALWAYS)
				FPRINTF (fp, ":%s", NAME_ALWAYS);
			FPRINTF (fp, "\n");
			break;
		case PR_CPI:
			print_sdn (fp, HEAD, prbufp->cpi);
			break;
		case PR_CS:
			print_l (fp, HEAD, prbufp->char_sets);
			break;
		case PR_ITYPES:
			print_l (fp, HEAD, prbufp->input_types);
			break;
		case PR_DEV:
			print_str (fp, HEAD, prbufp->device);
			break;
		case PR_DIAL:
			print_str (fp, HEAD, prbufp->dial_info);
			break;
		case PR_RECOV:
			print_str (fp, HEAD, prbufp->fault_rec);
			break;
		case PR_INTFC:
			print_str (fp, HEAD, prbufp->interface);
			break;
		case PR_LPI:
			print_sdn (fp, HEAD, prbufp->lpi);
			break;
		case PR_LEN:
			print_sdn (fp, HEAD, prbufp->plen);
			break;
		case PR_LOGIN:
			if (prbufp->login & LOG_IN)
				FPRINTF (fp, "%s\n", HEAD);
			break;
		case PR_PTYPE:
			if (!prbufp->printer_type || !*prbufp->printer_type)
				print_str (fp, HEAD, NAME_UNKNOWN);
			else
				print_str (fp, HEAD, prbufp->printer_type);
			break;
		case PR_REMOTE:
			print_str (fp, HEAD, prbufp->remote);
			break;
		case PR_SPEED:
			print_str (fp, HEAD, speed);
			break;
		case PR_STTY:
			print_str (fp, HEAD, stty);
			break;
		case PR_WIDTH:
			print_sdn (fp, HEAD, prbufp->pwid);
			break;
		}

	}
	if (stty)
		free (stty);
	if (ferror(fp)) {
		close_lpfile (fp);
		return (-1);
	}
	close_lpfile (fp);

	/*
	 * If we have a description of the printer,
	 * write it out to a separate file.
	 */
	if (prbufp->description) {

		if (!(path = getprinterfile(name, COMMENTFILE)))
			return (-1);

		if (dumpstring(path, prbufp->description) == -1) {
			free (path);
			return (-1);
		}
		free (path);
	
	}

	/*
	 * Now write out the alert condition.
	 */
	if (
		prbufp->fault_alert.shcmd
	     && putalert(Lp_A_Printers, name, &(prbufp->fault_alert)) == -1
	)
		return (-1);

	return (0);
}

/**
 ** print_sdn() - PRINT SCALED DECIMAL NUMBER WITH HEADER
 ** print_l() - PRINT (char **) LIST WITH HEADER
 ** print_str() - PRINT STRING WITH HEADER
 **/

static void		print_sdn (fp, head, sdn)
	FILE			*fp;
	char			*head;
	SCALED			sdn;
{
	if (sdn.val <= 0)
		return;

	FPRINTF (fp, "%s ", head);
	printsdn (fp, sdn);

	return;
}

static void		print_l (fp, head, list)
	FILE			*fp;
	char			*head,
				**list;
{
	if (!list || !*list)
		return;

	FPRINTF (fp, "%s ", head);
	printlist_setup (0, 0, LP_SEP, 0);
	printlist (fp, list);
	printlist_unsetup ();

	return;
}

static void		print_str (fp, head, str)
	FILE			*fp;
	char			*head,
				*str;
{
	if (!str || !*str)
		return;

	FPRINTF (fp, "%s %s\n", head, str);

	return;
}
