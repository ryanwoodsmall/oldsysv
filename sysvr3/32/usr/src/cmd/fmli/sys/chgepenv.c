/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/chgepenv.c	1.1"

#include	<stdio.h>
#include	"wish.h"


char *
chgepenv(name, value)
char	*name, *value;
{
	char dirpath[PATHSIZ];
	extern char	*Home;
	char	*strcat();
	char	*strcpy();
	char	*chgenv();

	return chgenv(strcat(strcpy(dirpath, Home), "/pref/.environ"), name, value);
}
