/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/requests/putrequest.c	1.5"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/utsname.h"

#include "lp.h"
#include "requests.h"

extern char		*malloc();

extern void		free();

extern struct {
	char			*v;
	short			len;
}			reqheadings[];

#define	WHEN		((
#define	USE		)?(
#define	OTERHWISE	):(
#define	NEHW		))

/**
 ** putrequest() - WRITE REQUEST STRUCTURE TO DISK FILE
 **/

int			putrequest (file, reqbufp)
	char			*file;
	REQUEST			*reqbufp;
{
	char			*user,
				**pp,
				*path;

	FILE			*fp;

	int			fld;

	/*
	 * First go through the structure and see if we have
	 * anything strange.
	 */
	if (
		reqbufp->copies <= 0
	     || !(reqbufp->destination)
	     || !reqbufp->file_list || !*(reqbufp->file_list)
	     || (reqbufp->actions & (ACT_MAIL|ACT_WRITE))
			&& (reqbufp->alert && *(reqbufp->alert))
	     || reqbufp->priority < -1 || 39 < reqbufp->priority
	) {
		errno = EINVAL;
		return (-1);
	}

#if	defined(NETWORKING)
	/*
	 * Make sure the "user" member has a machine name
	 * attached.
	 */
	if (!strchr((user = reqbufp->user), '!')) {

		struct utsname		ubuf;

		char			*me;

		(void)uname (&ubuf);
		if (!reqbufp->user)
			me = reqbufp->user;
		else
			me = strdup(getname());
		user = malloc(strlen(ubuf.nodename) + 1 + strlen(me) + 1);
		strcpy (user, ubuf.nodename);
		strcat (user, "!");
		strcat (user, me);

	}
#else
	user = reqbufp->user;
#endif

	/*
	 * Now open the file and write out the request.
	 */

	if (!Lp_Temp) {
		getpaths ();
		if (!Lp_Temp)
			return (-1);
	}
	if (!(path = makepath(Lp_Temp, file, (char *)0)))
		return (-1);
	if (!(fp = open_lpfile(path, "w", MODE_NOREAD))) {
		free (path);
		return (-1);
	}
	free (path);

	for (fld = 0; fld < RQ_MAX; fld++)  switch (fld) {

#define HEAD	reqheadings[fld].v

	case RQ_COPIES:
		FPRINTF (fp, "%s%d\n", HEAD, reqbufp->copies);
		break;
	case RQ_DEST:
		FPRINTF (fp, "%s%s\n", HEAD, reqbufp->destination);
		break;
	case RQ_FILE:
		for (pp = reqbufp->file_list; *pp; pp++)
			FPRINTF (fp, "%s%s\n", HEAD, *pp);
		break;
	case RQ_FORM:
		if (reqbufp->form)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->form);
		break;
	case RQ_HANDL:
		if ((reqbufp->actions & ACT_SPECIAL) == ACT_IMMEDIATE)
			FPRINTF (fp, "%s%s\n", HEAD, NAME_IMMEDIATE);
		else if ((reqbufp->actions & ACT_SPECIAL) == ACT_RESUME)
			FPRINTF (fp, "%s%s\n", HEAD, NAME_RESUME);
		else if ((reqbufp->actions & ACT_SPECIAL) == ACT_HOLD)
			FPRINTF (fp, "%s%s\n", HEAD, NAME_HOLD);
		break;
	case RQ_NOTIFY:
		if (reqbufp->actions & ACT_MAIL)
			FPRINTF (fp, "%sM\n", HEAD);
		else if (reqbufp->actions & ACT_WRITE)
			FPRINTF (fp, "%sW\n", HEAD);
		else if (reqbufp->alert && *(reqbufp->alert))
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->alert);
		break;
	case RQ_OPTS:
		if (reqbufp->options)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->options);
		break;
	case RQ_PRIOR:
		if (reqbufp->priority != -1)
			FPRINTF (fp, "%s%d\n", HEAD, reqbufp->priority);
		break;
	case RQ_PAGES:
		if (reqbufp->pages)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->pages);
		break;
	case RQ_CHARS:
		if (reqbufp->charset)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->charset);
		break;
	case RQ_TITLE:
		if (reqbufp->title)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->title);
		break;
	case RQ_MODES:
		if (reqbufp->modes)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->modes);
		break;
	case RQ_TYPE:
		if (reqbufp->input_type)
			FPRINTF (fp, "%s%s\n", HEAD, reqbufp->input_type);
		break;
	case RQ_USER:
		FPRINTF (fp, "%s%s\n", HEAD, user);
		break;
	case RQ_RAW:
		if (reqbufp->actions & ACT_RAW)
			FPRINTF (fp, "%s\n", HEAD);
		break;
	case RQ_FAST:
		if (reqbufp->actions & ACT_FAST)
			FPRINTF (fp, "%s\n", HEAD);
		break;
	case RQ_STAT:
		FPRINTF (fp, "%s%#6.4x\n", HEAD, reqbufp->outcome);
		break;
	}

	close_lpfile (fp);
	return (0);
}
