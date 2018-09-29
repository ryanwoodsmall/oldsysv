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
#include <sys/utsname.h>

static char Sccsid[] = "@(#)sysname.c	1.2";

/*
 * Return a pointer to a string
 * containing the nodename of the unix system.
 * Return 0 on error.
 */
char *
sysname()
{
	static struct utsname uinfo;

	if(uname(&uinfo) < 0)
		return(0);
	else
		return(uinfo.nodename);
}
