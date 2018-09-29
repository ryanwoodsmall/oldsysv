/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.oh:src/oh/if_exec.c	1.1"

#include <stdio.h>
#include <varargs.h>
#include "wish.h"
#include "terror.h"


int
IF_exec_open(argv)
char *argv[];
{
	(void) proc_openv(0, NULL, NULL, argv);
}
