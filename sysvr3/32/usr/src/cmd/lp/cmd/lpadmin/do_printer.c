/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpadmin/do_printer.c	1.8"

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "sys/types.h"

#include "lp.h"
#include "class.h"
#include "printers.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include	"oam.h"

#include "lpadmin.h"

extern	void	fromallclasses();

#if     !defined(PATH_MAX)
# define PATH_MAX       1024
#endif
#if     PATH_MAX < 1024
# undef PATH_MAX
# define PATH_MAX       1024
#endif

extern char		*malloc();

extern char		*label;

static void		configure_printer();

static char		*fullpath();

char			*nameit();

/**
 ** do_printer() - CREATE OR CHANGE PRINTER
 **/

void			do_printer ()
{
	int			rc;

	/*
	 * Set or change the printer configuration.
	 */
	if (strlen(modifications))
		configure_printer (modifications);

	/*
	 * Allow/deny forms.
	 */
	BEGIN_CRITICAL
		if (!oldp)
			if (allow_form_printer(getlist(NAME_NONE, "", ","), p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

		if (f_allow || f_deny) {
			if (f_allow && allow_form_printer(f_allow, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

			if (f_deny && deny_form_printer(f_deny, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}
		}
	END_CRITICAL

	/*
	 * Allow/deny users.
	 */
	BEGIN_CRITICAL
		if (!oldp)
			if (allow_user_printer(getlist(NAME_ALL, "", ","), p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

		if (u_allow || u_deny) {
			if (u_allow && allow_user_printer(u_allow, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}

			if (u_deny && deny_user_printer(u_deny, p) == -1) {
				LP_ERRMSG1 (ERROR, E_ADM_ACCESSINFO, PERROR);
				done(1);
			}
		}
	END_CRITICAL

	/*
	 * Tell the Spooler about the printer
	 */
	send_message(S_LOAD_PRINTER, p, "", "");
	rc = output(R_LOAD_PRINTER);

	switch (rc) {
	case MOK:
		break;

	case MNODEST:
	case MERRDEST:
		LP_ERRMSG (ERROR, E_ADM_ERRDEST);
		done (1);
		/*NOTREACHED*/

	case MNOSPACE:
		LP_ERRMSG (WARNING, E_ADM_NOPSPACE);
		break;

	case MNOPERM:	/* taken care of up front */
	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
		done (1);
		/*NOTREACHED*/
	}

	/*
	 * Now that the Spooler knows about the printer,
	 * we can do the balance of the changes.
	 */

	/*
	 * Mount or unmount form, print-wheel.
	 */
	if (M) 
		do_mount(p, (f? f : (char *)0), (S? *S : (char *)0));

	/*
	 * Display the alert type.
	 */
	if (A && STREQU(A, NAME_LIST)) {
		if (label)
			PRINTF ("Printer %s: ", label);
		printalert (stdout, &(oldp->fault_alert), 1);
	}

	/*
	 * -A quiet.
	 */
	if (A && STREQU(A, NAME_QUIET)) {

		send_message(S_QUIET_ALERT, p, (char *)QA_PRINTER, "");
		rc = output(R_QUIET_ALERT);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:	/* not quite, but not a lie either */
		case MERRDEST:
			LP_ERRMSG1 (WARNING, E_LP_NOQUIET, p);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}
	}

	/*
	 * Add printer p to class c
	 */
	if (c)  {
		CLASS			*pc,
					clsbuf;

		if (STREQU(c, NAME_ANY))
			c = NAME_ALL;

Loop:		if (!(pc = getclass(c))) {
			if (STREQU(c, NAME_ALL))
				goto Done;

			if (errno != ENOENT) {
				LP_ERRMSG2 (
					ERROR,
					E_LP_GETCLASS,
					c,
					PERROR
				);
				done (1);
			}

			/*
			 * Create the class
			 */
			clsbuf.name = strdup(c);
			clsbuf.members = 0;
			if (addlist(&clsbuf.members, p) == -1) {
				LP_ERRMSG (ERROR, E_LP_MALLOC);
				done (1);
			}
			pc = &clsbuf;

		} else if (searchlist(p, pc->members))
			LP_ERRMSG2 (WARNING, E_ADM_INCLASS, p, pc->name);

		else if (addlist(&pc->members, p) == -1) {
			LP_ERRMSG (ERROR, E_LP_MALLOC);
			done (1);
		}

		BEGIN_CRITICAL
			if (putclass(pc->name, pc) == -1) {
				LP_ERRMSG2 (
					ERROR,
					E_LP_PUTCLASS,
					pc->name,
					PERROR
				);
				done(1);
			}
		END_CRITICAL

		send_message (S_LOAD_CLASS, pc->name);
		rc = output(R_LOAD_CLASS);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:
		case MERRDEST:
			LP_ERRMSG (ERROR, E_ADM_ERRDEST);
			done (1);
			/*NOTREACHED*/

		case MNOSPACE:
			LP_ERRMSG (WARNING, E_ADM_NOCSPACE);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}

		if (STREQU(c, NAME_ALL))
			goto Loop;
	}
Done:
	/*
	 * Remove printer p from class r
	 */
	if (r) {
		if (STREQU(r, NAME_ALL) || STREQU(r, NAME_ANY))
			fromallclasses(p);
		else 
			fromclass(p, r);
	}

	return;
}

/**
 ** configure_printer() - SET OR CHANGE CONFIGURATION OF PRINTER
 **/

static void		configure_printer (list)
	char			*list;
{
	register PRINTER	*prbufp;

	PRINTER			printer_struct;

	char			type;


	if (oldp) {

		prbufp = oldp;

		if (!T)
			T = prbufp->printer_type;

		if (!i && !e && !m)
			/*
			 * Don't copy the original interface program
			 * again, but do keep the name of the original.
			 */
			ignprinter = BAD_INTERFACE;
		else
			ignprinter = 0;

	} else {
		prbufp = &printer_struct;
		prbufp->banner = BAN_ALWAYS;
		prbufp->cpi.val = 0;
		prbufp->cpi.sc = 0;
		prbufp->char_sets = 0;
		prbufp->input_types = 0;
		prbufp->device = 0;
		prbufp->dial_info = 0;
		prbufp->fault_rec = 0;
		prbufp->interface = makepath(Lp_Model, m, (char *)0);
		prbufp->lpi.val = 0;
		prbufp->lpi.sc = 0;
		prbufp->plen.val = 0;
		prbufp->plen.sc = 0;
		prbufp->login = 0;
		prbufp->printer_type = 0;
		prbufp->remote = 0;
		prbufp->speed = 0;
		prbufp->stty = 0;
		prbufp->pwid.val = 0;
		prbufp->pwid.sc = 0;
		prbufp->description = 0;
		if (!A)
			prbufp->fault_alert.shcmd = nameit(NAME_MAIL);
		prbufp->fault_alert.Q = 0;
		prbufp->fault_alert.W = 0;
	}

	while ((type = *list++) != '\0')  switch(type) {

	case 'A':
		if (STREQU(A, NAME_MAIL) || STREQU(A, NAME_WRITE))
			prbufp->fault_alert.shcmd = nameit(A);
		else if (!STREQU(A, NAME_QUIET))
			prbufp->fault_alert.shcmd = A;

		break;

	case 'b':
		prbufp->banner = banner;
		break;

	case 'c':
		prbufp->cpi = cpi_sdn;
		break;

	case 'D':
		prbufp->description = D;
		break;

	case 'e':
		prbufp->interface = makepath(
			Lp_A_Interfaces,
			e,
			(char *)0
		);
		break;

	case 'F':
		prbufp->fault_rec = F;
		break;

	case 'h':
		prbufp->login = 0;
		break;

	case 'i':
		prbufp->interface = fullpath(i);
		break;

	case 'I':
		prbufp->input_types = I;
		break;

	case 'l':
		prbufp->login = 1;
		break;

	case 'L':
		prbufp->plen = length_sdn;
		break;

	case 'm':
		prbufp->interface = makepath(Lp_Model,	m, (char *)0);
		break;

	case 'M':
		prbufp->lpi = lpi_sdn;
		break;

	case 's':
		prbufp->speed = 0;    /* lpadmin always defers to stty */
		prbufp->stty = stty;
		break;

	case 'S':
		if (!M)
			if (STREQU(*S, NAME_NONE))
				prbufp->char_sets = 0;
			else
				prbufp->char_sets = S;
		break;

	case 'T':
		prbufp->printer_type = T;
		break;

	case 'U':
		prbufp->dial_info = U;
		prbufp->device = 0;
		break;

	case 'v':
		prbufp->device = v;
		prbufp->dial_info = 0;
		break;

	case 'w':
		prbufp->pwid = width_sdn;
		break;

	case 'W':
		prbufp->fault_alert.W = W;
		break;

	}

	BEGIN_CRITICAL
		if (putprinter(p, prbufp) == -1) {
			if (
				errno == EINVAL
			     && (badprinter & BAD_INTERFACE)
			)
				LP_ERRMSG1 (
					ERROR,
					E_ADM_BADINTF,
					prbufp->interface
				);
			else
				LP_ERRMSG2 (
					ERROR,
					E_LP_PUTPRINTER,
					p,
					PERROR
				);
			done(1);
		}
	END_CRITICAL

	return;
}

/**
 ** fullpath()
 **/

static char		*fullpath (str)
	char			*str;
{
	register char		*cur_dir,
				*path;


	while (*str && *str == ' ')
		str++;
	if (*str == '/')
		return (str);

	if (!(cur_dir = malloc(PATH_MAX + 1)))
		return (str);

	curdir (cur_dir);
	path = makepath(cur_dir, str, (char *)0);

	/*
	 * Here we could be nice and strip out /./ and /../
	 * stuff, but it isn't necessary.
	 */

	return (path);
}

/**
 ** nameit() - ADD USER NAME TO COMMAND
 **/

char			*nameit (cmd)
	char			*cmd;
{
	register char		*nm = getname(),
				*copy = malloc(
					(unsigned) (strlen(cmd) + 1 + 
					strlen(nm) + 1)
	);

	(void) strcpy (copy, cmd);
	(void) strcat (copy, " ");
	(void) strcat (copy, nm);
	return (copy);
}
