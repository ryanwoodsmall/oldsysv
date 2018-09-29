/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/secure.c	1.6"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"
#include "sys/param.h"

#include "lpsched.h"

extern long		atol();

/**
 ** getsecure() - EXTRACT SECURE REQUEST STRUCTURE FROM DISK FILE
 **/

SECURE *
getsecure (file)
	char			*file;
{
	static SECURE		secbuf;

	char			buf[BUFSIZ],
				*path;

	FILE			*fp;

	int			fld;


	if (!Lp_Requests) {
		getpaths ();
		if (!Lp_Requests)
			return (0);
	}
	if (!(path = makepath(Lp_Requests, file, (char *)0)))
		return (0);

	if (!(fp = open_lpfile(path, "r"))) {
		free (path);
		return (0);
	}
	free (path);

	secbuf.user = 0;
	for (
		fld = 0;
		fld < SC_MAX && fgets(buf, BUFSIZ, fp) != NULL;
		fld++
	) {
		buf[strlen(buf) - 1] = 0;
		switch (fld) {

		case SC_REQID:
			secbuf.req_id = strdup(buf);
			break;

		case SC_UID:
			secbuf.uid = atoi(buf);
			break;

		case SC_USER:
			secbuf.user = strdup(buf);
			break;

		case SC_GID:
			secbuf.gid = atoi(buf);
			break;

		case SC_SIZE:
			secbuf.size = (size_t)atol(buf);
			break;

		case SC_DATE:
			secbuf.date = (time_t)atol(buf);
			break;

		}
	}
	if (ferror(fp) || fld != SC_MAX) {
		int			save_errno = errno;

		if (secbuf.user)
			free (secbuf.user);
		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	/*
	 * Now go through the structure and see if we have
	 * anything strange.
	 */
	if (
	        secbuf.uid > MAXUID
	     || !secbuf.user
	     || secbuf.gid > MAXUID 
	     || secbuf.size == 0
	     || secbuf.date <= 0
	) {
		if (secbuf.user)
			free (secbuf.user);
		errno = EBADF;
		return (0);
	}

	return (&secbuf);
}

/**
 ** putsecure() - WRITE SECURE REQUEST STRUCTURE TO DISK FILE
 **/

int
putsecure (file, secbufp)
	char			*file;
	SECURE			*secbufp;
{
	char			*path;

	FILE			*fp;

	int			fld;

	if (!Lp_Requests) {
		getpaths ();
		if (!Lp_Requests)
			return (-1);
	}
	if (!(path = makepath(Lp_Requests, file, (char *)0)))
		return (-1);
	if (!(fp = open_lpfile(path, "w", MODE_NOREAD))) {
		free (path);
		return (-1);
	}
	free (path);

	if (
		!secbufp->req_id ||
		!secbufp->user
	)
		return (-1);

	for (fld = 0; fld < SC_MAX; fld++)

		switch (fld) {

		case SC_REQID:
			FPRINTF (fp, "%s\n", secbufp->req_id);
			break;

		case SC_UID:
			FPRINTF (fp, "%d\n", secbufp->uid);
			break;

		case SC_USER:
			FPRINTF (fp, "%s\n", secbufp->user);
			break;

		case SC_GID:
			FPRINTF (fp, "%d\n", secbufp->gid);
			break;

		case SC_SIZE:
			FPRINTF (fp, "%lu\n", secbufp->size);
			break;

		case SC_DATE:
			FPRINTF (fp, "%ld\n", secbufp->date);
			break;
		}


	close_lpfile (fp);

	return (0);
}
