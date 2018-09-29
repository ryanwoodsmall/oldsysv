/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/alerts.c	1.8"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "unistd.h"

#if	!defined(PATH_MAX)
# define PATH_MAX	1024
#endif
#if	PATH_MAX < 1024
# undef PATH_MAX
# define PATH_MAX	1024
#endif

#include "lp.h"

extern char		**environ;

extern void		free();

extern long		ulimit();

#define	WHEN		((
#define	USE		)?(
#define	OTHERWISE	):(
#define	NEHW		))

static void		envlist();

/*
 * We recognize the following key phrases in the alert prototype
 * file, and replace them with appropriate values.
 */
#define NALRT_KEYS	7
# define ALRT_ENV		0
# define ALRT_PWD		1
# define ALRT_ULIMIT		2
# define ALRT_UMASK		3
# define ALRT_INTERVAL		4
# define ALRT_CMD		5
# define ALRT_USER		6

static struct {
	char			*v;
	short			len;
}			shell_keys[NALRT_KEYS] = {
#define	ENTRY(X)	X, sizeof(X)-1
	ENTRY("-ENVIRONMENT-"),
	ENTRY("-PWD-"),
	ENTRY("-ULIMIT-"),
	ENTRY("-UMASK-"),
	ENTRY("-INTERVAL-"),
	ENTRY("-CMD-"),
	ENTRY("-USER-"),
};

/*
 * These are used to bracket the administrator's command, so that
 * we can find it easily. We're out of luck if the administrator
 * includes an identical phrase in his or her command.
 */
#define ALRT_CMDSTART "## YOUR COMMAND STARTS HERE -- DON'T TOUCH ABOVE!!"
#define ALRT_CMDEND   "## YOUR COMMAND ENDS HERE -- DON'T TOUCH BELOW!!"

/**
 ** putalert() - WRITE ALERT TO FILES
 **/

int			putalert (parent, name, alertp)
	char			*parent,
				*name;
	FALERT			*alertp;
{
	char			*path,
				cur_dir[PATH_MAX + 1],
				buf[BUFSIZ];

	int			cur_umask;

	FILE			*fpout,
				*fpin;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (!alertp->shcmd) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(alertp->shcmd, NAME_NONE))
		return (delalert(parent, name));

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (-1);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		free (path);
		return (-1);
	}
	free (path);

	/*
	 * First, the shell command file.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (-1);

	if (!(fpout = open_lpfile(path, "w", MODE_NOEXEC))) {
		free (path);
		return (-1);
	}
	free (path);

	/*
	 * We use a prototype file to build the shell command,
	 * so that the alerts are easily customized. The shell
	 * is expected to handle repeat alerts and failed alerts,
	 * because the Spooler doesn't. Also, the Spooler runs
	 * each alert with the UID and GID of the administrator
	 * who defined the alert. Otherwise, anything goes.
	 */

	if (!Lp_Bin) {
		getpaths ();
		if (!Lp_Bin)
			return (-1);
	}
	if (!(path = makepath(Lp_Bin, ALERTPROTOFILE, (char *)0)))
		return (-1);

	if (!(fpin = open_lpfile(path, "r"))) {
		free (path);
		return (-1);
	}
	free (path);

	while (fgets(buf, BUFSIZ, fpin)) {
		int			key;
		char			*cp,
					*dash;

		cp = buf;
		while ((dash = strchr(cp, '-'))) {

		    *dash = 0;
		    fputs (cp, fpout);
		    *(cp = dash) = '-';

		    for (key = 0; key < NALRT_KEYS; key++)
			if (STRNEQU(
				cp,
				shell_keys[key].v,
				shell_keys[key].len
			)) {
				register char	*newline =
						(cp != buf)? "\n" : "";

				cp += shell_keys[key].len;

				switch (key) {

				case ALRT_ENV:
					fprintf (fpout, newline);
					envlist (fpout, environ);
					break;

				case ALRT_PWD:
					curdir (cur_dir);
					fprintf (fpout, "%s", cur_dir);
					break;

				case ALRT_ULIMIT:
					fprintf (fpout, "%ld", ulimit(1, (long)0));
					break;

				case ALRT_UMASK:
					umask (cur_umask = umask(0));
					fprintf (fpout, "%03o", cur_umask);
					break;

				case ALRT_INTERVAL:
					fprintf (fpout, "%ld", (long)alertp->W);
					break;

				case ALRT_CMD:
					fprintf (fpout, newline);
					fprintf (fpout, "%s\n", ALRT_CMDSTART);
					fprintf (fpout, "%s\n", alertp->shcmd);
					fprintf (fpout, "%s\n", ALRT_CMDEND);
					break;

				case ALRT_USER:
					fprintf (fpout, "%s", getname());
					break;

				}

				break;
			}
		    if (key >= NALRT_KEYS)
			fputc (*cp++, fpout);

		}
		fputs (cp, fpout);

	}
	if (feof(fpout) || ferror(fpout) || ferror(fpin)) {
		int			save_errno = errno;

		close_lpfile (fpin);
		close_lpfile (fpout);
		errno = save_errno;
		return (-1);
	}
	close_lpfile (fpin);
	close_lpfile (fpout);

	/*
	 * Next, the variables file.
	 */

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (-1);

	if (!(fpout = open_lpfile(path, "w", MODE_NOREAD))) {
		free (path);
		return (-1);
	}
	free (path);

	fprintf (fpout, "%d\n", alertp->Q > 0? alertp->Q : 1);
	fprintf (fpout, "%d\n", alertp->W >= 0? alertp->W : 0);

	close_lpfile (fpout);

	return (0);
}

/**
 ** getalert() - EXTRACT ALERT FROM FILES
 **/

FALERT			*getalert (parent, name)
	char			*parent,
				*name;
{
	static FALERT		alert;

	register char		*path;

	char			buf[BUFSIZ];

	FILE			*fp;

	int			len;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (0);
	}

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (0);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		free (path);
		return (0);
	}
	free (path);

	/*
	 * First, the shell command file.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (0);

	if (!(fp = open_lpfile(path, "r"))) {
		free (path);
		return (0);
	}
	free (path);

	/*
	 * Skip over environment setting stuff, while loop, etc.,
	 * to find the beginning of the command.
	 */
	while (
		fgets(buf, BUFSIZ, fp)
	     && !STRNEQU(buf, ALRT_CMDSTART, sizeof(ALRT_CMDSTART)-1)
	)
		;
	if (feof(fp) || ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}

	alert.shcmd = sop_up_rest(fp, ALRT_CMDEND);
	if (!alert.shcmd) {
		close_lpfile (fp);
		return (0);
	}

	/*
	 * Drop terminating newline.
	 */
	if (alert.shcmd[(len = strlen(alert.shcmd)) - 1] == '\n')
		alert.shcmd[len - 1] = 0;

	close_lpfile (fp);

	/*
	 * Next, the variables file.
	 */

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (0);

	if (!(fp = open_lpfile(path, "r"))) {
		free (path);
		return (0);
	}
	free (path);

	(void)fgets (buf, BUFSIZ, fp);
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	alert.Q = atoi(buf);

	(void)fgets (buf, BUFSIZ, fp);
	if (ferror(fp)) {
		int			save_errno = errno;

		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	alert.W = atoi(buf);

	close_lpfile (fp);

	return (&alert);
}

/**
 ** delalert() - DELETE ALERT FILES
 **/

int			delalert (parent, name)
	char			*parent,
				*name;
{
	char			*path;


	if (!parent || !*parent || !name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * See if the form/printer/print-wheel exists.
	 */

	if (!(path = makepath(parent, name, (char *)0)))
		return (-1);

	if (Access(path, F_OK) == -1) {
		if (errno == ENOENT)
			errno = ENOTDIR; /* not quite, but what else? */
		free (path);
		return (-1);
	}
	free (path);

	/*
	 * Remove the two files.
	 */

	if (!(path = makepath(parent, name, ALERTSHFILE, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	if (!(path = makepath(parent, name, ALERTVARSFILE, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		free (path);
		return (-1);
	}
	free (path);

	return (0);
}

/**
 ** envlist() - PRINT OUT ENVIRONMENT LIST SAFELY
 **/

static void		envlist (fp, list)
	register FILE		*fp;
	register char		**list;
{
	register char		*env,
				*value;

	if (!list || !*list)
		return;

	while ((env = *list++)) {
		if (!(value = strchr(env, '=')))
			continue;
		*value++ = 0;
		if (!strchr(value, '\''))
			fprintf (fp, "export %s; %s='%s'\n", env, env, value);
		*--value = '=';
	}
}

/**
 ** printalert() - PRINT ALERT DESCRIPTION
 **/

void			printalert (fp, alertp, isfault)
	register FILE		*fp;
	register FALERT		*alertp;
	int			isfault;
{
	if (!alertp->shcmd) {
		if (isfault)
			FPRINTF (fp, "On fault: no alert\n");
		else
			FPRINTF (fp, "No alert\n");

	} else {
		register char	*copy = strdup(alertp->shcmd),
				*cp;

		if (isfault)
			FPRINTF (fp, "On fault: ");
		else
			if (alertp->Q > 1)
				FPRINTF (
					fp,
					"When %d are queued: ",
					alertp->Q
				);
			else
				FPRINTF (fp, "Upon any being queued: ");

		if (copy && (cp = strchr(copy, ' ')))
			while (*cp == ' ')
				*cp++ = 0;

		if (
			copy
		     && syn_name(cp)
		     && (
				STREQU(copy, NAME_WRITE)
			     || STREQU(copy, NAME_MAIL)
			)
		)
			FPRINTF (fp, "%s to %s ", copy, cp);
		else
			FPRINTF (fp, "alert with \"%s\" ", alertp->shcmd);

		if (alertp->W > 0)
			FPRINTF (fp, "every %d minutes\n", alertp->W);
		else
			FPRINTF (fp, "once\n");

		free (copy);
	}
	return;
}
