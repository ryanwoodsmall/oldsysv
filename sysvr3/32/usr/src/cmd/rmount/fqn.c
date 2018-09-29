/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	fqn - Convert a resource name into a fully qualified name.
	If a name does not contain a domain name, prepend one.
	The resultant name is placed in a supplied buffer buf.
	fqn does not complain.  If it can't get a domain name, it doesn't.
*/

#ident	"@(#)rmount:fqn.c	1.1"
#include <nserve.h>
#include <sys/rfsys.h>
#include <string.h>

char *
fqn(name, buf)
char *name, *buf;
{
	static char dname[MAXDNAME] = "";
	static char *dnamep;

	if (strchr(name, '.'))	/* check if the name is already fully qualified */
		return strcpy(buf, name);
	if (!dname[0])		/* dname not initialized yet */
		if (rfsys(RF_GETDNAME, dname, MAXDNAME) < 0)
			return strcpy(buf, name);
		else {
			for (dnamep=dname; *dnamep; ++dnamep) ;
			*dnamep++ = '.';
			*dnamep++ = '\0';
		}
	strcpy(buf, dname);
	strncat(buf, name, 32);		/* 32 is max size of entries in rmnttab */
	return buf;
}
