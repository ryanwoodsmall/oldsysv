/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.sys:src/sys/evstr.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"eval.h"

char *
evalstr(s)
char	*s;
{
	IOSTRUCT	*in;
	IOSTRUCT	*out;

	in = io_open(EV_USE_STRING | EV_READONLY, s);
	out = io_open(EV_USE_STRING, NULL);
	while (eval(in, out, EV_TOKEN) != ET_EOF)
		;
	s = io_string(out);
	io_close(out);
	return s;
}
