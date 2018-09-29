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
/* @(#)uucpname.c	1.6 */
#include "uucp.h"

#ifdef UNAME
#include <sys/utsname.h>
#endif

/*
 * get the uucp name
 * return:
 *	none
 */
uucpname(name)
register char *name;
{
	register char *s, *d;

#ifdef UNAME
	struct utsname utsn;

	uname(&utsn);
	s = utsn.nodename;
#endif

#ifndef UNAME
	s = MYNAME;
#endif

	d = name;
	while ((*d = *s++) && d < name + SYSNSIZE)
		d++;
	*(name + SYSNSIZE) = '\0';
	return;
}
