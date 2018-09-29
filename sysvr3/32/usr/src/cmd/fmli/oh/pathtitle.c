/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.oh:src/oh/pathtitle.c	1.1"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"

#define MAX_FOLTITLE	(40)

/*
 * note:  This subroutine depends on the fact that its argument is
 * a legal UNIX path, i.e., slash separated strings with no more
 * than 14 characters in each string.
 */
char *
path_to_title(s, pre)
char	*s;
char	*pre;
{
	static char	title[MAX_FOLTITLE+1];
	register int	len;
	extern char	*Home;
	char	*strchr();

	if (pre == NULL)
		pre = Home;
	len = strlen(pre);
	if (strncmp(s, pre, len) == 0 && s[len] == '/')	/* clip off pre */
		s += len + 1;
	if ((len = strlen(s)) >= MAX_FOLTITLE) {
		register char	*p;
		register int	n;

		/* replace part of it by ... */
		p = s + MAX_FOLTITLE / 2;
		while (*p != '/' && p > s)
			p--;
		n = ++p - s;
		strncpy(title, s, n);
		strcpy(&title[n], "...");
		if (p = strchr(&s[len - MAX_FOLTITLE / 2 + 3], '/'))
			strcat(title, p);
	}
	else
		strcpy(title, s);
	return title;
}
