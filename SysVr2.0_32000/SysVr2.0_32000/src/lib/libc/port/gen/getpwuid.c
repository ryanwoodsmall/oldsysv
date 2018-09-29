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
/*	@(#)getpwuid.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <pwd.h>

extern struct passwd *getpwent();
extern void setpwent(), endpwent();

struct passwd *
getpwuid(uid)
register int uid;
{
	register struct passwd *p;

	setpwent();
	while((p = getpwent()) && p->pw_uid != uid)
		;
	endpwent();
	return(p);
}
