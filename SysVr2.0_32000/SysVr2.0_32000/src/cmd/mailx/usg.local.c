/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Local routines that are installation dependent.
 */

#include "rcv.h"

/* static char *SccsId = "@(#)usg.local.c	1.1"; */

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).  In Standard UNIX, it is in /usr/mail/name.
 */

findmail()
{
	register char *cp;

	cp = copy("/usr/mail/", mailname);
	copy(myname, cp);
	if (isdir(mailname)) {
		stradd(mailname, '/');
		strcat(mailname, myname);
	}
}

/*
 * Get rid of the queued mail.
 */

demail()
{
	char cmd[BUFSIZ];

	close(creat(mailname, 0660));
	/*
	 * rmmail is a setgid program usually needed to remove a file
	 * from the /usr/mail directory. It only removes files that
	 * are empty and have suitable permissions.
	 * If it doesn't succeed in removing the file, at least the
	 * file has been set to 0 length.
	 */
	if (value("keep") == NOSTR) {
		sprintf(cmd,"%s %s",libpath("rmmail"),mailname);
		system(cmd);
	}
}

/*
 * Discover user login name.
 */

username(uid, namebuf)
	char namebuf[];
{
	register char *np;

	if (uid == getuid() && (np = getenv("LOGNAME")) != NOSTR) {
		strncpy(namebuf, np, PATHSIZE);
		return(0);
	}
	return(getname(uid, namebuf));
}
