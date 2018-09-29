/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/stream.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"token.h"

token
stream(t, s)
register token t;
register token (*s[])();
{
	register int	i;

	for (i = 0; s[i]; i++)
		if ((t = (*(s[i]))(t)) == TOK_NOP)
			break;
	return t;
}
