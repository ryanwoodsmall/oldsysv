/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/getname.c	1.3"
/*
 *	getname(name)  --  get logname
 *
 *		getname tries to find the user's logname from:
 *			${LOGNAME}, if set and if it is telling the truth
 *			/etc/passwd, otherwise
 *
 *		The logname is returned as the value of the function.
 *
 *		Getname returns the user's user id converted to ASCII
 *		for unknown lognames.
 *
 */

#include "string.h"
#include "pwd.h"
#include "errno.h"

extern char		*getenv();

extern struct passwd	*getpwnam(),
			*getpwuid();

extern void		endpwent();

extern unsigned short	getuid();

char			*getname ()
{
	int			uid;
	struct passwd		*p;
	static char		*logname	= 0;
	char			*l;

	if (logname)
		return (logname);

	uid = getuid();

	setpwent ();
	if (
		!(l = getenv("LOGNAME"))
	     || !(p = getpwnam(l))
	     || p->pw_uid != uid
	)
		if ((p = getpwuid(uid)))
			l = p->pw_name;
		else
			l = 0;
	endpwent ();

	if (l)
		logname = strdup(l);
	else {
		if (uid > 0) {
			extern char		*malloc();

			logname = malloc(10 + 1);
			if (logname)
				sprintf (logname, "%d", uid);
		}
	}

	if (!logname)
		errno = ENOMEM;
	return (logname);
}
