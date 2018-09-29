/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/sendmail.c	1.2"
/* sendmail(user, msg) -- send msg to user's mailbox */

#include "stdio.h"
#include "pwd.h"

#include "lp.h"

extern char		*malloc();

extern void		free();

void			sendmail (user, msg)
	char			*user,
				*msg;
{
	FILE			*pfile;

	char			*mailcmd;

	if (isnumber(user))
		return;

	if ((mailcmd = malloc(strlen(MAIL) + 1 + strlen(user) + 1))) {
		sprintf (mailcmd, "%s %s", MAIL, user);
		if ((pfile = popen(mailcmd, "w"))) {
			FPRINTF (pfile, "%s\n", msg);
			pclose (pfile);
		}
		free (mailcmd);
	}
	return;
}
