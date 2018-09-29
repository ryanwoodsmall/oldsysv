/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/printers/getprinter.c	1.9"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "fs.h"
#include "printers.h"

extern void		free();

extern double		strtod();

extern struct {
	char			*v;
	short			len,
				okremote;
}			prtrheadings[];

static SCALED		get_sdn();

/**
 ** getprinter() - EXTRACT PRINTER STRUCTURE FROM DISK FILE
 **/

PRINTER			*getprinter (name)
	char			*name;
{
	static long		lastdir		= -1;

	static PRINTER		prbuf;

	char			buf[BUFSIZ];

	int			fld;

	FILE			*fp;

	FALERT			*pa;

	register char		*p,
				**pp,
				***ppp,
				*path;


	if (!name || !*name) {
		errno = EINVAL;
		return (0);
	}

	if (!Lp_A_Printers) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers)
			return (0);
	}

	/*
	 * Getting ``all''? If so, jump into the directory
	 * wherever we left off.
	 */
	if (STREQU(NAME_ALL, name)) {
		if (!(name = next_dir(Lp_A_Printers, &lastdir)))
			return (0);
	} else
		lastdir = -1;

	/*
	 * Get the printer configuration information.
	 */

	path = getprinterfile(name, CONFIGFILE);
	if (!path)
		return (0);

	if (!(fp = open_lpfile(path, "r"))) {
		free (path);
		return (0);
	}
	free (path);

	(void)memset ((char *)&prbuf, 0, sizeof(prbuf));
	prbuf.name = strdup(name);

	while (fgets(buf, BUFSIZ, fp) != NULL) {

		buf[strlen(buf) - 1] = 0;

		for (fld = 0; fld < PR_MAX; fld++)
			if (
				prtrheadings[fld].v
			     && prtrheadings[fld].len
			     && STRNEQU(
					buf,
					prtrheadings[fld].v,
					prtrheadings[fld].len
				)
			) {
				p = buf + prtrheadings[fld].len;
				while (*p && *p == ' ')
					p++;
				break;
			}

		if (fld >= PR_MAX) {
			freeprinter (&prbuf);
			close_lpfile (fp);
			errno = EBADF;
			return (0);
		}

		switch (fld) {

		case PR_BAN:
			if ((pp = getlist(p, LP_WS, ":"))) {
				if (pp[0] && STREQU(pp[0], NAME_OFF))
					prbuf.banner |= BAN_OFF;
				if (pp[1] && CS_STREQU(pp[1], NAME_ALWAYS))
					prbuf.banner |= BAN_ALWAYS;
				freelist (pp);
			}
			break;
		case PR_LOGIN:
			prbuf.login = LOG_IN;
			break;
		case PR_CPI:
			prbuf.cpi = get_sdn(p);
			break;
		case PR_LPI:
			prbuf.lpi = get_sdn(p);
			break;
		case PR_LEN:
			prbuf.plen = get_sdn(p);
			break;
		case PR_WIDTH:
			prbuf.pwid = get_sdn(p);
			break;
		case PR_CS:
			ppp = &(prbuf.char_sets);
			goto CharStarStar;
		case PR_ITYPES:
			ppp = &(prbuf.input_types);
CharStarStar:		if (*ppp)
				freelist (*ppp);
			*ppp = getlist(p, LP_WS, LP_SEP);
			break;
		case PR_DEV:
			pp = &(prbuf.device);
			goto CharStar;
		case PR_DIAL:
			pp = &(prbuf.dial_info);
			goto CharStar;
		case PR_RECOV:
			pp = &(prbuf.fault_rec);
			goto CharStar;
		case PR_INTFC:
			pp = &(prbuf.interface);
			goto CharStar;
		case PR_PTYPE:
			pp = &(prbuf.printer_type);
			goto CharStar;
		case PR_REMOTE:
			pp = &(prbuf.remote);
			goto CharStar;
		case PR_SPEED:
			pp = &(prbuf.speed);
			goto CharStar;
		case PR_STTY:
			pp = &(prbuf.stty);
CharStar:		if (*pp)
				free (*pp);
			*pp = strdup(p);
			break;
		}


	}
	if (ferror(fp)) {
		int			save_errno = errno;

		freeprinter (&prbuf);
		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	/*
	 * Get the printer description (if it exists).
	 */
	if (!(path = getprinterfile(name, COMMENTFILE)))
		return (0);
	if (!(prbuf.description = loadstring(path)) && errno != ENOENT) {
		free (path);
		freeprinter (&prbuf);
		return (0);
	}
	free (path);

	/*
	 * Get the information for the alert. Don't fail if we can't
	 * read it because of access permission UNLESS we're "root"
	 * or "lp"
	 */
	if (!(pa = getalert(Lp_A_Printers, name))) {
		if (
			errno != ENOENT
		     && (
				errno != EACCES
			     || !getpid()		  /* we be root */
			     || STREQU(getname(), LPUSER) /* we be lp   */
			)
		) {
			freeprinter (&prbuf);
			return (0);
		}
	} else
		prbuf.fault_alert = *pa;

	/*
	 * Now go through the structure and see if we have
	 * anything strange.
	 */
	if (!okprinter(name, &prbuf, 0)) {
		freeprinter (&prbuf);
		errno = EBADF;
		return (0);
	}

	/*
	 * Guarantee some return values won't be null or empty.
	 */
	if (!prbuf.printer_type || !*prbuf.printer_type) {
		prbuf.printer_type = strdup(NAME_UNKNOWN);
		prbuf.daisy = 0;
	} else {
		tidbit (prbuf.printer_type, "daisy", &prbuf.daisy);
		if (prbuf.daisy == -1)
			prbuf.daisy = 0;
	}
	if (!prbuf.input_types)
		prbuf.input_types = getlist(NAME_SIMPLE, LP_WS, LP_SEP);

	return (&prbuf);
}

/**
 ** get_sdn() - PARSE SCALED DECIMAL NUMBER
 **/

static SCALED		get_sdn (str)
	char			*str;
{
	SCALED			sdn;

	char			*rest;

	sdn.val = strtod(str, &rest);

	while (*rest && *rest == ' ')
		rest++;
	switch (*rest) {
	case 'i':
	case 'c':
	case 0:
		sdn.sc = *rest;
		break;
	default:
		sdn.sc = 0;
		break;
	}

	return (sdn);
}
