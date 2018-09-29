/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *      All Rights Reserved
 */

#ident	"@(#)vm:oh/suffuncs.c	1.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>


char *
rem_suffix(str, num)
char *str; int num;
{
	static char buf[DIRSIZ+1];

	strcpy(buf, str);
	buf[strlen(str)-num] = '\0';
	return((char *)buf);
}

char *
add_suffix(str, suf)
char *str, *suf;
{
	static char buf[DIRSIZ+1];
	char *strcat();

	if (strlen(str) +strlen(suf) > DIRSIZ)
		return((char *)NULL);

	return(strcat(strcpy(buf, str), suf));
}


int
has_suffix(str,suf)
char *str, *suf;
{
	char *p;
	p = str + strlen(str) - strlen(suf);
	if (strcmp(p, suf) == 0) {
		return(1);
	} else {
		return(0);
	}
}
