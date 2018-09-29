/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm:sys/expand.c	1.1"

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	"wish.h"
#include	"moremacros.h"

char *
expand(name)
char	*name;
{
	char	buf[BUFSIZ];
	char	*pexpand();

	(void) pexpand(buf, name, '\0');
	return strsave(buf);
}

static char *
pexpand(buf, name, eos)
char	*buf;
char	*name;
char	eos;
{
	register char	delim;
	register char	*src;
	register char	*dst;
	register char	*file;
	char	fbuf[PATHSIZ];
	char	*anyenv();
	char	*getepenv();

	dst = buf;
	src = name;
	while (*src && *src != eos) {
		if (*src == '\\') {
			++src;
			*dst++ = *src++;
		}
		else if (*src == '$') {
			register char	*start;

			if ((delim = (*++src == '{') ? '}' : '\0'))
				start = ++src;
			else
				start = src;
			file = NULL;
			if (*src == '(') {
				src = pexpand(fbuf, ++src, ')');
				if (*src) {
					start = ++src;
					file = fbuf;
				}
			}
			if (isalpha(*src)) {
				register char	*p;
				register char	savechar;

				while (isalpha(*src) || isdigit(*src) || *src == '_')
					src++;
				savechar = *src;
				*src = '\0';
				if ((p = (file ? anyenv(file, start) : getepenv(start))) == NULL) {
					if (delim) {
						if ((*src = savechar) == ':' && *++src == '-')
							while (*++src && *src != delim)
								*dst++ = *src;
					}
					else
						*src = savechar;
				}
				else {
					*src = savechar;
					strcpy(dst, p);
					dst += strlen(p);
					free(p);
				}
				if (delim)
					while (*src && *src++ != delim)
						;
			}
		}
		else
			*dst++ = *src++;
	}
	*dst = '\0';
	return src;
}
