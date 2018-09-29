/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/onexit.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

extern int	(**Onexit)();

void
onexit(func)
int	(*func)();
{
	Onexit = (int (**)()) array_check_append(sizeof(int (*)()), (struct v_array *) Onexit, &func);
}
