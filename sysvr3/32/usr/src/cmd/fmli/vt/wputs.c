/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.vt:src/vt/wputs.c	1.3"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"attrs.h"

void
wputs(s)
char	*s;
{
	register char	*p;

	for (p = s; *p; p++) {
		if (*p == '\\') {
			switch(*(++p)) {
			case 'b':
				*p = '\b';
				break;
			case 'n':
				*p = '\n';
				break;
			case 't':
				*p = '\t';
				break;
			case 'r':
				*p = '\r';
				break;
			case '\0':
				return;
			}
		}
		wputchar(*p, ATTR_NORMAL);
	}
}
